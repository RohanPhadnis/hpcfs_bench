#!/bin/bash
#
# Test 01: Metadata Benchmarks
# - Uses 'mdtest' to measure file/dir create, stat, and remove operations.
#

source ./config.sh || { echo "Failed to load config.sh from $(pwd)"; exit 1; }
set -e

LOG_FILE="$LOG_DIR/01_metadata_bench.log"
TEST_DIR="$MOUNT_POINT/metadata_test_01"
TEST_DIR_TREE="$MOUNT_POINT/metadata_test_02"

echo "Test 01: Metadata Benchmarks. Logging to $LOG_FILE"
echo "  -> Using mdtest. This may take a few minutes."
exec > >(tee -a "$LOG_FILE") 2>&1

# Test 1: 10,000 file creates/stats/removes in one directory
echo "--- Test 1: Single-Directory Stress (10k files) ---"
mdtest -n 10000 -i 3 -d "$TEST_DIR"
rm -rf "$TEST_DIR"

# Test 2: 100,000 files in a tree structure (10 levels deep)
echo "--- Test 2: Tree-Structure Stress (100k files) ---"
# 10 * 10 * 10 * 10 * 10 = 100k files
mdtest -n 10 -i 3 -d "$TEST_DIR_TREE" -F -z 5 -I 10
rm -rf "$TEST_DIR_TREE"

# Test 3: Real-world 'find' (readdir/getattr performance)
echo "--- Test 3: 'find' on $SMALL_FILE_COUNT small files ---"
# Time how long it takes to traverse the small file dataset
(time find "$SMALL_FILE_DIR" -type f | wc -l)

echo "Metadata test complete."
