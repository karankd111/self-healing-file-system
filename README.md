# self-healing-file-system
A mini filesystem with self-healing capabilities that detects and repairs corrupted blocks using redundancy and checksums.

Features :
Automatic Corruption Detection - CRC32 checksums on every block
Self-Healing Recovery - Automatically repairs corrupted blocks from replicas
Block-Based Storage - Fixed 4KB blocks with built-in redundancy
Triple Replication - Every block stored 3 times for fault tolerance
Recovery Logging - Detailed logs of all repair operations
Interactive Shell - Command-line interface for filesystem operations
Zero Dependencies - Pure C++17, no external libraries required

Prerequisites :
Compiler: GCC 7+ or Clang 5+ with C++17 support
OS: Linux or macOS
Tools: Make

Quick Start :
1. Clone the Repository
bashgit clone https://github.com/yourusername/self-healing-fs.git
cd self-healing-fs
2. Build the Project
bashmake
3. Run the Filesystem
bash./build/shfs
4. Try It Out
bashshfs> format                              # Initialize filesystem
shfs> mkdir /documents                    # Create directory
shfs> write /documents/hello.txt Hello World!  # Write file
shfs> ls /documents                       # List directory
shfs> read /documents/hello.txt           # Read file
shfs> fsck                                # Check filesystem health
shfs> exit                                # Exit

Testing Corruption Recovery :
Automatic Test Script
bash./test_corruption.sh
Manual Testing

Write a file:

bash   shfs> write /test.txt This is important data

Exit and corrupt a block manually:

bash   # Find and corrupt a block file
   dd if=/dev/urandom of=./data/fs_storage/replica_0/block_0.blk bs=100 count=1 conv=notrunc

Read the file (triggers auto-recovery):

bash   shfs> read /test.txt
   # Should detect corruption and automatically repair from replica_1 or replica_2

Check the recovery log:

bash   cat ./data/fs_storage/recovery.log
Expected output:
[Tue Dec 09 10:30:45 2025] Block 0: 1 corrupted replica(s) detected
[Tue Dec 09 10:30:45 2025] Block 0: Replica 0 RECOVERED from replica 1

Example Session :
bash$ ./build/shfs
=================================
  Self-Healing File System v1.0
=================================

Type 'help' for commands, 'exit' to quit

shfs> format
Storage initialized at: ./data/fs_storage
Replicas: 3
Filesystem formatted successfully

shfs> mkdir /projects
Directory created: /projects

shfs> mkdir /projects/cpp
Directory created: /projects/cpp

shfs> write /projects/cpp/readme.txt This is a self-healing filesystem demo
File written: /projects/cpp/readme.txt (39 bytes, 1 blocks)

shfs> ls /
  projects/

shfs> ls /projects
  cpp/

shfs> read /projects/cpp/readme.txt
Content: This is a self-healing filesystem demo

shfs> fsck
===== Starting full filesystem check =====
===== Check complete: 1 blocks, 0 corrupted, 0 recovered, 0 unrecoverable =====

shfs> exit
Goodbye!

Build Options:
Clean Build
bashmake clean
make
Run Directly
bashmake run
Debug Build (with symbols)
bashmake CXXFLAGS="-std=c++17 -Wall -Wextra -g -I./include"

How It Works :
Block Structure
Each block is 4KB (4096 bytes):

4088 bytes: Actual data
8 bytes: CRC32 checksum

Replication Strategy
Every block is written to 3 replicas:
Block 0 → replica_0/block_0.blk
       → replica_1/block_0.blk
       → replica_2/block_0.blk
Corruption Detection
On every read operation:

Read block from primary replica (replica_0)
Compute CRC32 of data
Compare with stored checksum
If mismatch detected → trigger recovery

Self-Healing Recovery
When corruption is detected:

Check all replicas for the corrupted block
Find a replica with valid checksum
Copy valid block to corrupted replica(s)
Log the recovery operation
Resume normal operation

Recovery Decision Matrix
Valid ReplicasCorruptedActionResult30None Healthy21Repair 1 Recovered12Repair 2 Recovered03None Data Loss

Performance Characteristics :

Block Size: 4KB (configurable at compile time)
Replication Factor: 3x (200% storage overhead)
Write Performance: O(n/B × R) where n=data size, B=block size, R=replicas
Read Performance: O(k) where k=number of blocks
Recovery Performance: O(R × B) per block

Security Considerations :
This is an educational project. Not recommended for production use.

CRC32 detects accidental corruption, not malicious tampering
No encryption or authentication
No access control or permissions
Metadata stored in memory only (not persistent)

Known Limitations :
No Persistence: Metadata (directory structure) lost on exit
Single-threaded: No concurrent access support
In-memory Metadata: Directory tree not saved to disk
No Compression: Raw data storage only
Fixed Block Size: 4KB blocks for all files
No Deduplication: Identical data stored multiple times

Troubleshooting
Build Errors
Error: filesystem header not found
bash# Ensure C++17 support
g++ --version  # Should be 7.0 or higher
Error: library 'stdc++fs' not found (macOS)

Already fixed in Makefile for macOS/Clang
If still occurs, remove -lstdc++fs from LDFLAGS

Runtime Issues
Error: Cannot create directories
bash# Check permissions
mkdir -p ./data/fs_storage
chmod 755 ./data/fs_storage
Recovery not working
bash# Check recovery log
cat ./data/fs_storage/recovery.log

# Verify replicas exist
ls -la ./data/fs_storage/replica_*/

