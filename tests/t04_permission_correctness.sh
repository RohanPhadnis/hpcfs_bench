#!/bin/bash
#
# Test 04: Permissions & ACLs (Multi-Node)
# - Verifies POSIX permissions and ACLs are consistent across nodes.
#

source ./config.sh || { echo "Failed to load config.sh from $(pwd)"; exit 1; }
set -e

LOG_FILE="$LOG_DIR/04_permissions_correctness.log"
TEST_DIR="$MOUNT_POINT/permissions_test_04"
TEST_FILE="$TEST_DIR/test_file.txt"
rm -rf "$TEST_DIR" && mkdir -p "$TEST_DIR"
touch "$TEST_FILE"

echo "Test 04: Permissions & ACLs. Logging to $LOG_FILE"
exec > >(tee -a "$LOG_FILE") 2>&1

# Test 1: Basic POSIX (chmod)
echo "--- Test 1: POSIX Permissions (chmod) ---"
chmod 600 "$TEST_FILE"
echo "  -> Set $TEST_FILE to 600 (rw-------)"

# Check from other nodes
for node in $CLIENT_NODES; do
    echo "  -> Checking POSIX denial from $node as user $TEST_USER_B..."
    # This command should fail
    ssh "$node" "su -c 'cat $TEST_FILE' $TEST_USER_B" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  -> PASS: Node $node correctly denied read for $TEST_USER_B."
    else
        echo "  -> FAIL: Node $node ALLOWED read for $TEST_USER_B. Breach!"
        exit 1
    fi
done

# Test 2: POSIX ACLs (setfacl)
echo "--- Test 2: POSIX ACLs (setfacl) ---"
setfacl -m u:"$TEST_USER_B":r "$TEST_FILE"
echo "  -> Added ACL: user $TEST_USER_B can read $TEST_FILE"

# Check from other nodes
for node in $CLIENT_NODES; do
    echo "  -> Checking ACL grant from $node as user $TEST_USER_B..."
    # This command should now SUCCEED
    ssh "$node" "su -c 'cat $TEST_FILE' $TEST_USER_B" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "  -> PASS: Node $node correctly allowed ACL read for $TEST_USER_B."
    else
        echo "  -> FAIL: Node $node DENIED read for $TEST_USER_B despite ACL."
        exit 1
    fi
done

rm -rf "$TEST_DIR"
echo "Permissions test complete."
