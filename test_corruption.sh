#!/bin/bash

echo "1. Starting filesystem..."
./build/shfs << EOF
format
mkdir /test
write /test/data.txt This is important data that must survive corruption!
exit
EOF

echo -e "\n2. Corrupting a block..."
# Find a block file and corrupt it
BLOCK_FILE=$(find ./data/fs_storage/replica_0 -name "block_*.blk" | head -n 1)
echo "Corrupting: $BLOCK_FILE"
dd if=/dev/urandom of="$BLOCK_FILE" bs=100 count=1 conv=notrunc 2>/dev/null

echo -e "\n3. Reading corrupted file (should trigger auto-recovery)..."
./build/shfs << EOF
read /test/data.txt
exit
EOF

echo -e "\n4. Running fsck..."
./build/shfs << EOF
fsck
exit
EOF