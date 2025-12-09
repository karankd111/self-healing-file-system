#include "../include/block.h"
#include <cstring>

// CRC32 lookup table
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

void init_crc32_table() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
        crc32_table[i] = crc;
    }
    crc32_table_initialized = true;
}

uint32_t crc32(const uint8_t* data, size_t length) {
    if (!crc32_table_initialized)
        init_crc32_table();
    
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return ~crc;
}

Block::Block() : checksum(0) {
    clear();
}

void Block::computeChecksum() {
    checksum = crc32(data, DATA_SIZE);
}

bool Block::verifyChecksum() const {
    return checksum == crc32(data, DATA_SIZE);
}

void Block::clear() {
    memset(data, 0, DATA_SIZE);
    checksum = 0;
}