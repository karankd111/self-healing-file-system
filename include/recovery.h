#ifndef RECOVERY_H
#define RECOVERY_H

#include "storage.h"
#include <string>
#include <fstream>

struct RecoveryStats {
    size_t blocks_checked = 0;
    size_t corrupted_blocks = 0;
    size_t recovered_blocks = 0;
    size_t unrecoverable_blocks = 0;
};

class RecoveryManager {
private:
    BlockStorage& storage;
    std::ofstream log_file;
    
    void log(const std::string& message);
    
public:
    RecoveryManager(BlockStorage& storage, const std::string& log_path);
    ~RecoveryManager();
    
    bool checkAndRepairBlock(size_t block_id);
    RecoveryStats checkAndRepairAll();
    bool verifyBlock(size_t block_id, size_t replica);
};

#endif