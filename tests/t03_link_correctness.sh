#!/bin/bash
#
# Test 03: Link Correctness (Multi-Node)
# - Verifies hardlinks and symlinks created on one node are
#   resolved correctly on other nodes.
# - This is the key "Conda Env" test.
#

source ./config.sh || { echo "Failed to load config.sh from $(pwd)"; exit 1; }
set -e

LOG_FILE="$LOG_DIR/03_link_correctness.log"
TEST_DIR="$MOUNT_POINT/link_test_03"
rm -rf "$TEST_DIR" && mkdir -p "$TEST_DIR"

echo "Test 03: Link Correctness (Multi-Node). Logging to $LOG_FILE"
exec > >(tee -a "$LOG_FILE") 2>&1

# Test 1: Hard Links
echo "--- Test 1: Hard Link Correctness ---"
echo "hardlink_data_v1" > "$TEST_DIR/hard_orig"
ln "$TEST_DIR/hard_orig" "$TEST_DIR/hard_link"

# Check inodes locally
INODE1=$(ls -i "$TEST_DIR/hard_orig" | awk '{print $1}')
INODE2=$(ls -i "$TEST_DIR/hard_link" | awk '{print $1}')

if [ "$INODE1" != "$INODE2" ]; then
    echo "  -> FAIL: Local hardlink inodes do not match ($INODE1 != $INODE2)"
    exit 1
else
    echo "  -> PASS: Local hardlink inodes match ($INODE1)"
fi

# Check hardlink data from all client nodes
for node in $CLIENT_NODES; do
    echo "  -> Checking hardlink data from $node..."
    CONTENT=$(ssh "$node" "cat $TEST_DIR/hard_link" 2>/dev/null)
    if [ "$CONTENT" != "hardlink_data_v1" ]; then
        echo "  -> FAIL: Node $node read incorrect hardlink data: '$CONTENT'"
        exit 1
    else
        echo "  -> PASS: Node $node read hardlink data correctly."
    fi
done

# Test 2: Symbolic Links
echo "--- Test 2: Symbolic Link Correctness ---"
echo "symlink_data_v2" > "$TEST_DIR/sym_target"
ln -s "sym_target" "$TEST_DIR/sym_link" # Use a relative link

# Check symlink data from all client nodes
for node in $CLIENT_NODES; do
    echo "  -> Checking symlink data from $node..."
    # We must `cd` into the directory for the relative link to work
    CONTENT=$(ssh "$node" "cd $TEST_DIR && cat sym_link" 2>/dev/null)
    if [ "$CONTENT" != "symlink_data_v2" ]; then
        echo "  -> FAIL: Node $node failed to resolve symlink: '$CONTENT'"
        exit 1
    else
        echo "  -> PASS: Node $node resolved symlink correctly."
    fi
done

# Test 3: The "Conda Env" Test (Real-world symlinks)
echo "--- Test 3: Real-World Conda Env Test ---"
if [ ! -d "$CONDA_ENV_DEST" ]; then
    echo "  -> FAIL: Conda env not found at $CONDA_ENV_DEST. Run setup script."
    exit 1
fi

for node in $CLIENT_NODES; do
    echo "  -> Checking conda env from $node..."
    # This command will fail if symlinks inside the env are broken
    VERSION=$(ssh "$node" "$CONDA_ENV_DEST/bin/python --version" 2>&1)
    if [[ "$VERSION" == *"Python 3.9"* ]]; then
        echo "  -> PASS: Node $node successfully executed python: $VERSION"
    else
        echo "  -> FAIL: Node $node FAILED to execute python. Error: $VERSION"
        exit 1
    fi
done

rm -rf "$TEST_DIR"
echo "Link correctness test complete."
