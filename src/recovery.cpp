#include "../include/recovery.h"
#include <iostream>
#include <chrono>
#include <iomanip>

RecoveryManager::RecoveryManager(BlockStorage& storage, const std::string& log_path)
    : storage(storage) {
    log_file.open(log_path, std::ios::app);
    log("Recovery manager initialized");
}

RecoveryManager::~RecoveryManager() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

void RecoveryManager::log(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::string timestamp = std::ctime(&time);
    timestamp.pop_back(); // Remove newline
    
    std::string log_msg = "[" + timestamp + "] " + message;
    
    if (log_file.is_open()) {
        log_file << log_msg << std::endl;
    }
    std::cout << log_msg << std::endl;
}

bool RecoveryManager::verifyBlock(size_t block_id, size_t replica) {
    Block block;
    if (!storage.readBlock(block_id, replica, block)) {
        return false;
    }
    return block.verifyChecksum();
}

bool RecoveryManager::checkAndRepairBlock(size_t block_id) {
    std::vector<size_t> valid_replicas;
    std::vector<size_t> corrupted_replicas;
    
    // Check all replicas
    for (size_t replica = 0; replica < storage.getNumReplicas(); replica++) {
        if (!storage.blockExists(block_id, replica)) {
            corrupted_replicas.push_back(replica);
            continue;
        }
        
        if (verifyBlock(block_id, replica)) {
            valid_replicas.push_back(replica);
        } else {
            corrupted_replicas.push_back(replica);
        }
    }
    
    // If no corruption, we're done
    if (corrupted_replicas.empty()) {
        return true;
    }
    
    log("Block " + std::to_string(block_id) + ": " + 
        std::to_string(corrupted_replicas.size()) + " corrupted replica(s) detected");
    
    // If no valid replicas, cannot recover
    if (valid_replicas.empty()) {
        log("Block " + std::to_string(block_id) + ": UNRECOVERABLE - no valid replicas");
        return false;
    }
    
    // Recover from first valid replica
    Block valid_block;
    storage.readBlock(block_id, valid_replicas[0], valid_block);
    
    // Overwrite corrupted replicas
    for (size_t replica : corrupted_replicas) {
        std::string path = storage.getBlockPath(replica, block_id);
        std::ofstream file(path, std::ios::binary);
        if (file) {
            file.write(reinterpret_cast<const char*>(&valid_block), sizeof(Block));
            file.close();
            log("Block " + std::to_string(block_id) + ": Replica " + 
                std::to_string(replica) + " RECOVERED from replica " + 
                std::to_string(valid_replicas[0]));
        }
    }
    
    return true;
}

RecoveryStats RecoveryManager::checkAndRepairAll() {
    RecoveryStats stats;
    log("===== Starting full filesystem check =====");
    
    std::vector<size_t> block_ids = storage.getAllBlockIds();
    stats.blocks_checked = block_ids.size();
    
    for (size_t block_id : block_ids) {
        bool had_corruption = false;
        
        for (size_t replica = 0; replica < storage.getNumReplicas(); replica++) {
            if (!verifyBlock(block_id, replica)) {
                had_corruption = true;
                break;
            }
        }
        
        if (had_corruption) {
            stats.corrupted_blocks++;
            if (checkAndRepairBlock(block_id)) {
                stats.recovered_blocks++;
            } else {
                stats.unrecoverable_blocks++;
            }
        }
    }
    
    log("===== Check complete: " + std::to_string(stats.blocks_checked) + " blocks, " +
        std::to_string(stats.corrupted_blocks) + " corrupted, " +
        std::to_string(stats.recovered_blocks) + " recovered, " +
        std::to_string(stats.unrecoverable_blocks) + " unrecoverable =====");
    
    return stats;
}