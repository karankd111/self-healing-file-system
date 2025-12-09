#include "../include/block.h"
#include <iostream>
#include <cstring>

int main() {
    Block block;
    
    // Write some data
    const char* test_data = "Hello, Self-Healing FS!";
    memcpy(block.data, test_data, strlen(test_data));
    
    // Compute checksum
    block.computeChecksum();
    std::cout << "Checksum: " << block.checksum << std::endl;
    
    // Verify
    if (block.verifyChecksum()) {
        std::cout << "✓ Checksum valid" << std::endl;
    }
    
    // Corrupt data
    block.data[0] = 'X';
    if (!block.verifyChecksum()) {
        std::cout << "✓ Corruption detected!" << std::endl;
    }
    
    return 0;
}