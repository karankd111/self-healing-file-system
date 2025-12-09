#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "storage.h"
#include "recovery.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

enum class NodeType { FILE, DIRECTORY };

struct INode {
    std::string name;
    NodeType type;
    std::vector<size_t> block_ids; // For files
    std::map<std::string, std::shared_ptr<INode>> children; // For directories
    
    INode(const std::string& n, NodeType t) : name(n), type(t) {}
};

class FileSystem {
private:
    BlockStorage storage;
    RecoveryManager recovery;
    std::shared_ptr<INode> root;
    size_t next_block_id;
    
    std::shared_ptr<INode> findNode(const std::string& path);
    std::vector<std::string> splitPath(const std::string& path);
    size_t allocateBlock();
    
public:
    FileSystem(const std::string& storage_path);
    
    bool format();
    bool mkdir(const std::string& path);
    bool writeFile(const std::string& path, const std::string& data);
    bool readFile(const std::string& path, std::string& data);
    bool deleteFile(const std::string& path);
    std::vector<std::string> ls(const std::string& path);
    
    bool fsck() {
        recovery.checkAndRepairAll();
        return true;
    }
    
    bool recover(size_t block_id) {
        return recovery.checkAndRepairBlock(block_id);
    }
};

#endif