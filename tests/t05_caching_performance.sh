#!/bin/bash
#
# Test 05: Caching Performance (2nd Time Access)
# - Measures read performance to differentiate OS cache vs. Filesystem cache.
#
# !! This test requires 'sudo' to drop the OS page cache. !!
#

source ./config.sh || { echo "Failed to load config.sh from $(pwd)"; exit 1; }
set -e

LOG_FILE="$LOG_DIR/05_caching_perf.log"

echo "Test 05: Caching Performance. Logging to $LOG_FILE"
echo "  -> This test requires sudo to drop OS caches."
exec > >(tee -a "$LOG_FILE") 2>&1

if [[ $EUID -eq 0 ]]; then
   echo "  -> Running as root."
else
   echo "  -> Requesting sudo to drop caches..."
   sudo -v # Ask for password upfront
fi

# Function to clear OS page cache
clear_cache() {
    echo "  -> Dropping OS page cache..."
    sync
    sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    sleep 2
}

READ_CMD="dd if=$LARGE_FILE_32G of=/dev/null bs=1M status=progress"

# --- Test 1: Cold Read ---
echo "--- Test 1: Cold Read (Empty Caches) ---"
clear_cache
(time $READ_CMD) 2>&1
echo "  -> Cold Read complete."

# --- Test 2: Hot Read (OS Page Cache) ---
echo "--- Test 2: Hot Read (Data in OS Cache) ---"
# DO NOT drop caches
(time $READ_CMD) 2>&1
echo "  -> Hot Read complete."

# --- Test 3: Warm Read (Filesystem Cache) ---
echo "--- Test 3: Warm Read (OS Cache dropped, FS cache active) ---"
clear_cache
(time $READ_CMD) 2>&1
echo "  -> Warm Read complete."

echo ""
echo "Caching Test Analysis:"
echo " - Compare 'real' time for Test 1 (Cold) vs. Test 3 (Warm)."
echo " - If Time 3 < Time 1, the filesystem provides effective caching."
echo " - Test 2 (Hot) shows the theoretical max speed from OS RAM."
echo "Caching test complete."
