source ./config.sh || { echo "Failed to load config.sh from $(pwd)"; exit 1; }
set -e

LOG_FILE="$LOG_DIR/02_data_io_bench.log"
FIO_OUT_DIR="$LOG_DIR/fio_results"
FIO_WRITE_FILE="$MOUNT_POINT/fio_temp_write.bin"
mkdir -p "$FIO_OUT_DIR"

echo "Test 02: Data I/O Benchmarks. Logging to $LOG_FILE"
echo "  -> Using fio. This will take several minutes."
exec > >(tee -a "$LOG_FILE") 2>&1

# Test 1: Large file sequential read (Throughput)
echo "--- Test 1: Sequential Read (32GB, 1M block) ---"
fio --name=seq-read \
    --filename="$LARGE_FILE_32G" \
    --rw=read --bs=1M --direct=1 \
    --size="${LARGE_FILE_SIZE_GB}G" \
    --output-format=json --output="$FIO_OUT_DIR/seq_read.json"

# Test 2: Large file sequential write (Throughput)
echo "--- Test 2: Sequential Write (32GB, 1M block) ---"
fio --name=seq-write \
    --filename="$FIO_WRITE_FILE" \
    --rw=write --bs=1M --direct=1 \
    --size="${LARGE_FILE_SIZE_GB}G" \
    --output-format=json --output="$FIO_OUT_DIR/seq_write.json"
rm -f "$FIO_WRITE_FILE" # Cleanup

# Test 3: Small file random read (IOPS)
echo "--- Test 3: Random Read IOPS (4k block, 10G total) ---"
fio --name=rand-read-iops \
    --directory="$SMALL_FILE_DIR" \
    --rw=randread --bs=4k --direct=1 \
    --size=10G --numjobs=8 --thread \
    --output-format=json --output="$FIO_OUT_DIR/rand_read_iops.json"

echo "Data I/O test complete. JSON results are in $FIO_OUT_DIR"
