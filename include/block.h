#ifndef BLOCK_H
#define BLOCK_H

#include <cstdint>
#include <vector>
#include <string>

// Fixed block size (4KB)
constexpr size_t BLOCK_SIZE = 4096;
constexpr size_t DATA_SIZE = BLOCK_SIZE - sizeof(uint32_t); // Reserve space for CRC

// Represents a single block with data + checksum
struct Block {
    uint8_t data[DATA_SIZE];
    uint32_t checksum;
    
    Block();
    void computeChecksum();
    bool verifyChecksum() const;
    void clear();
};

// CRC32 calculation
uint32_t crc32(const uint8_t* data, size_t length);

#endif