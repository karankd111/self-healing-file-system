#ifndef STORAGE_H
#define STORAGE_H

#include "block.h"
#include <string>
#include <vector>

// Forward declaration
class RecoveryManager;

class BlockStorage {
private:
    std::string base_path;
    size_t num_replicas;
    
    std::string getBlockPath(size_t replica, size_t block_id) const;
    std::string getReplicaPath(size_t replica) const;
    
public:
    // Make RecoveryManager a friend so it can access private methods
    friend class RecoveryManager;
    
    BlockStorage(const std::string& path, size_t replicas = 3);
    
    bool initialize();
    bool writeBlock(size_t block_id, const Block& block);
    bool readBlock(size_t block_id, size_t replica, Block& block) const;
    bool blockExists(size_t block_id, size_t replica) const;
    
    size_t getNumReplicas() const { return num_replicas; }
    std::vector<size_t> getAllBlockIds() const;
};

#endif