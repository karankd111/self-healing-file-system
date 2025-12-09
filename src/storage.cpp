#include "../include/storage.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

BlockStorage::BlockStorage(const std::string& path, size_t replicas)
    : base_path(path), num_replicas(replicas) {}

std::string BlockStorage::getReplicaPath(size_t replica) const {
    return base_path + "/replica_" + std::to_string(replica);
}

std::string BlockStorage::getBlockPath(size_t replica, size_t block_id) const {
    return getReplicaPath(replica) + "/block_" + std::to_string(block_id) + ".blk";
}

bool BlockStorage::initialize() {
    try {
        // Create base directory
        fs::create_directories(base_path);
        
        // Create replica directories
        for (size_t i = 0; i < num_replicas; i++) {
            fs::create_directories(getReplicaPath(i));
        }
        
        std::cout << "Storage initialized at: " << base_path << std::endl;
        std::cout << "Replicas: " << num_replicas << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize storage: " << e.what() << std::endl;
        return false;
    }
}

bool BlockStorage::writeBlock(size_t block_id, const Block& block) {
    // Write to all replicas
    for (size_t replica = 0; replica < num_replicas; replica++) {
        std::string path = getBlockPath(replica, block_id);
        
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to write block " << block_id 
                      << " to replica " << replica << std::endl;
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(&block), sizeof(Block));
        file.close();
    }
    
    return true;
}

bool BlockStorage::readBlock(size_t block_id, size_t replica, Block& block) const {
    std::string path = getBlockPath(replica, block_id);
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    file.read(reinterpret_cast<char*>(&block), sizeof(Block));
    return file.good();
}

bool BlockStorage::blockExists(size_t block_id, size_t replica) const {
    return fs::exists(getBlockPath(replica, block_id));
}

std::vector<size_t> BlockStorage::getAllBlockIds() const {
    std::vector<size_t> block_ids;
    
    // Scan first replica
    if (!fs::exists(getReplicaPath(0))) return block_ids;
    
    for (const auto& entry : fs::directory_iterator(getReplicaPath(0))) {
        std::string filename = entry.path().filename().string();
        if (filename.find("block_") == 0 && filename.find(".blk") != std::string::npos) {
            size_t id = std::stoul(filename.substr(6));
            block_ids.push_back(id);
        }
    }
    
    return block_ids;
}