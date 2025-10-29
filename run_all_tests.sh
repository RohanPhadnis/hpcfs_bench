#!/bin/bash
#
# Master Test Runner
#
# This script executes all test modules found in the ./tests/ directory
# in alphanumeric order.
#

# Load configuration and exit on error
source ./config.sh || { echo "Failed to load config.sh"; exit 1; }
set -e # Exit on any test failure

TEST_DIR="$(pwd)/tests"

echo "=========================================================="
echo "Starting Filesystem Test Suite"
echo "Mount Point: $MOUNT_POINT"
echo "Log Directory: $LOG_DIR"
echo "=========================================================="

for test_script in $(ls "$TEST_DIR"/*.sh | sort); do
    TEST_NAME=$(basename "$test_script")
    echo ""
    echo "--- [RUNNING] $TEST_NAME ---"
    
    # Run the test
    bash "$test_script"
    
    echo "--- [PASSED] $TEST_NAME ---"
done

echo ""
echo "=========================================================="
echo "Test Suite Completed Successfully."
echo "All results are in $LOG_DIR"
echo "=========================================================="
