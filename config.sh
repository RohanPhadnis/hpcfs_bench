#!/bin/bash
#
# Central Configuration File for the Filesystem Test Suite
#
# -----------------------------------------------------------------------------
# !! This is the only file you should need to edit. !!
# -----------------------------------------------------------------------------

# --- Core Paths ---
# The *exact* mount point of your distributed filesystem.
# This MUST be the same path on all nodes.
export MOUNT_POINT="/mnt/hpc-fs"

# --- Concurrency & Multi-Node Settings ---
# A space-separated list of *other* client nodes for pdsh to use.
# Do not include the node you are running from (localhost).
export CLIENT_NODES="node-b node-c node-d"

# A secondary user that exists on all nodes for permissions tests.
export TEST_USER_B="other_user"


# --- Logging ---
# A new timestamped log directory is created for each run.
export LOG_DIR_BASE="$(pwd)/logs"
export LOG_DIR="$LOG_DIR_BASE/$(date +%Y-%m-%d_%H-%M-%S)"
mkdir -p "$LOG_DIR"
echo "Logging results to $LOG_DIR"


# --- Data Plane Definitions ---
# These variables define the test data structure.
# They are used by setup_data_plane.sh (to create) and the
# test scripts (to read).
export DATA_DIR="$MOUNT_POINT/data_plane"

# Large files (for caching & throughput)
export LARGE_FILE_DIR="$DATA_DIR/large_files"
export LARGE_FILE_32G="$LARGE_FILE_DIR/file_32G.bin"
export LARGE_FILE_SIZE_GB=32

# Small files (for metadata & random I/O)
export DATASET_DIR="$DATA_DIR/datasets"
export SMALL_FILE_DIR="$DATASET_DIR/small_files_text"
export SMALL_FILE_COUNT=100000
export SMALL_FILE_SIZE_KB=4
export TARBALL_PATH="$DATASET_DIR/large_tarball.tar"

# Conda/Symlink test (for link correctness)
export CONDA_DIR="$DATA_DIR/conda_envs"
export CONDA_ENV_SRC="/tmp/my-test-env" # Local scratch path for creation
export CONDA_ENV_DEST="$CONDA_DIR/test_env_node_A"


# --- Tool Paths (Optional) ---
# If fio/mdtest/pdsh are not in the default $PATH, specify them here.
# export FIO_BIN="/usr/bin/fio"
# export MDTEST_BIN="/opt/ior/bin/mdtest"
# export PDSH_BIN="/usr/bin/pdsh"

# Set PDSH to use SSH
export PDSH_RCMD_TYPE=ssh
