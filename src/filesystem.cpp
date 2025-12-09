#include "../include/filesystem.h"
#include <sstream>
#include <cstring>
#include <iostream>

FileSystem::FileSystem(const std::string& storage_path)
    : storage(storage_path, 3),
      recovery(storage, storage_path + "/recovery.log"),
      next_block_id(0) {
    root = std::make_shared<INode>("/", NodeType::DIRECTORY);
}

std::vector<std::string> FileSystem::splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    
    return parts;
}

std::shared_ptr<INode> FileSystem::findNode(const std::string& path) {
    if (path == "/") return root;
    
    std::vector<std::string> parts = splitPath(path);
    std::shared_ptr<INode> current = root;
    
    for (const auto& part : parts) {
        if (current->type != NodeType::DIRECTORY) return nullptr;
        
        auto it = current->children.find(part);
        if (it == current->children.end()) return nullptr;
        
        current = it->second;
    }
    
    return current;
}

size_t FileSystem::allocateBlock() {
    return next_block_id++;
}

bool FileSystem::format() {
    if (!storage.initialize()) {
        return false;
    }
    
    root = std::make_shared<INode>("/", NodeType::DIRECTORY);
    next_block_id = 0;
    
    std::cout << "Filesystem formatted successfully" << std::endl;
    return true;
}

bool FileSystem::mkdir(const std::string& path) {
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) return false;
    
    std::string parent_path = "/";
    for (size_t i = 0; i < parts.size() - 1; i++) {
        parent_path += parts[i] + "/";
    }
    
    std::shared_ptr<INode> parent = findNode(parent_path);
    if (!parent || parent->type != NodeType::DIRECTORY) {
        std::cerr << "Parent directory not found" << std::endl;
        return false;
    }
    
    std::string dir_name = parts.back();
    if (parent->children.find(dir_name) != parent->children.end()) {
        std::cerr << "Directory already exists" << std::endl;
        return false;
    }
    
    parent->children[dir_name] = std::make_shared<INode>(dir_name, NodeType::DIRECTORY);
    std::cout << "Directory created: " << path << std::endl;
    return true;
}

bool FileSystem::writeFile(const std::string& path, const std::string& data) {
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) return false;
    
    std::string parent_path = "/";
    for (size_t i = 0; i < parts.size() - 1; i++) {
        parent_path += parts[i] + "/";
    }
    
    std::shared_ptr<INode> parent = findNode(parent_path);
    if (!parent || parent->type != NodeType::DIRECTORY) {
        std::cerr << "Parent directory not found" << std::endl;
        return false;
    }
    
    std::string file_name = parts.back();
    std::shared_ptr<INode> file_node;
    
    // Create or overwrite file
    auto it = parent->children.find(file_name);
    if (it != parent->children.end() && it->second->type == NodeType::FILE) {
        file_node = it->second;
        file_node->block_ids.clear(); // Overwrite
    } else {
        file_node = std::make_shared<INode>(file_name, NodeType::FILE);
        parent->children[file_name] = file_node;
    }
    
    // Write data in blocks
    size_t offset = 0;
    while (offset < data.size()) {
        Block block;
        size_t to_copy = std::min(DATA_SIZE, data.size() - offset);
        memcpy(block.data, data.c_str() + offset, to_copy);
        block.computeChecksum();
        
        size_t block_id = allocateBlock();
        if (!storage.writeBlock(block_id, block)) {
            std::cerr << "Failed to write block" << std::endl;
            return false;
        }
        
        file_node->block_ids.push_back(block_id);
        offset += to_copy;
    }
    
    std::cout << "File written: " << path << " (" << data.size() 
              << " bytes, " << file_node->block_ids.size() << " blocks)" << std::endl;
    return true;
}

bool FileSystem::readFile(const std::string& path, std::string& data) {
    std::shared_ptr<INode> node = findNode(path);
    if (!node || node->type != NodeType::FILE) {
        std::cerr << "File not found" << std::endl;
        return false;
    }
    
    data.clear();
    
    for (size_t block_id : node->block_ids) {
        Block block;
        if (!storage.readBlock(block_id, 0, block)) {
            std::cerr << "Failed to read block " << block_id << std::endl;
            return false;
        }
        
        if (!block.verifyChecksum()) {
            std::cerr << "Block " << block_id << " corrupted, attempting recovery..." << std::endl;
            if (!recovery.checkAndRepairBlock(block_id)) {
                std::cerr << "Recovery failed" << std::endl;
                return false;
            }
            // Re-read after recovery
            storage.readBlock(block_id, 0, block);
        }
        
        data.append(reinterpret_cast<char*>(block.data), DATA_SIZE);
    }
    
    // Trim null bytes
    data.erase(std::find(data.begin(), data.end(), '\0'), data.end());
    
    return true;
}

std::vector<std::string> FileSystem::ls(const std::string& path) {
    std::vector<std::string> entries;
    
    std::shared_ptr<INode> node = findNode(path);
    if (!node || node->type != NodeType::DIRECTORY) {
        return entries;
    }
    
    for (const auto& child : node->children) {
        std::string entry = child.first;
        if (child.second->type == NodeType::DIRECTORY) {
            entry += "/";
        }
        entries.push_back(entry);
    }
    
    return entries;
}

bool FileSystem::deleteFile(const std::string& path) {
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) return false;
    
    std::string parent_path = "/";
    for (size_t i = 0; i < parts.size() - 1; i++) {
        parent_path += parts[i] + "/";
    }
    
    std::shared_ptr<INode> parent = findNode(parent_path);
    if (!parent) return false;
    
    std::string name = parts.back();
    parent->children.erase(name);
    
    std::cout << "Deleted: " << path << std::endl;
    return true;
}