#!/bin/bash
#
# Test 06: Concurrency & Stress Tests
# - Uses 'fio' and 'mdtest' with 'pdsh' for multi-client,
#   multi-threaded workloads.
#

source ./config.sh || { echo "Failed to load config.sh from $(pwd)"; exit 1; }
set -e

LOG_FILE="$LOG_DIR/06_concurrency_stress.log"
FIO_OUT_DIR="$LOG_DIR/fio_results"
SHARED_MD_DIR="$MOUNT_POINT/concurrency_mdtest_06"
CLIENT_HOSTS=$(echo "$CLIENT_NODES" | tr ' ' ',') # Convert to comma-sep list

mkdir -p "$FIO_OUT_DIR"
mkdir -p "$SHARED_MD_DIR"

echo "Test 06: Concurrency Stress Test. Logging to $LOG_FILE"
echo "  -> Using pdsh to target: $CLIENT_HOSTS"
exec > >(tee -a "$LOG_FILE") 2>&1

# Test 1: Multi-threaded (single-client) random write
echo "--- Test 1: Single-Client, Multi-Threaded Random Write (16 threads) ---"
fio --name=multithread-randwrite \
    --directory="$MOUNT_POINT" \
    --rw=randwrite --bs=128k --direct=1 \
    --size=1G --numjobs=16 --thread \
    --output-format=json --output="$FIO_OUT_DIR/multithread_write.json"

# Test 2: Multi-client, shared read (Thundering Herd)
echo "--- Test 2: Multi-Client, Shared Read (Thundering Herd) ---"
pdsh -w "$CLIENT_HOSTS" \
    "fio --name=multi-client-read \
    --filename=$LARGE_FILE_32G \
    --rw=read --bs=1M --direct=1 --size=10G \
    --output-format=json --output=$LOG_DIR/fio_client_read_\$HOSTNAME.json"

echo "  -> Multi-client read test launched. Check JSON outputs for aggregate."

# Test 3: Multi-client metadata stress (shared directory)
echo "--- Test 3: Multi-Client, Shared Directory Metadata Stress ---"
# This is a major lock contention test.
pdsh -w "$CLIENT_HOSTS" \
    "mdtest -n 1000 -i 2 -d $SHARED_MD_DIR/node_\$HOSTNAME"
    
echo "  -> Multi-client metadata test complete."

rm -rf "$SHARED_MD_DIR"
echo "Concurrency test complete."
