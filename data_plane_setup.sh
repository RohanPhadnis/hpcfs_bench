#!/bin/bash
#
# Idempotent Data Plane Setup Script
#
# This script populates the target filesystem with a diverse set of test data.
# It checks if files or directories already exist before creating them to
# save time on subsequent runs.
#

set -euo pipefail # Exit on error

# --- Configuration ---
MOUNT_POINT="/mnt/hpc-fs"
DATA_DIR="$MOUNT_POINT/data_plane"

LARGE_FILE_DIR="$DATA_DIR/large_files"
LARGE_FILE_32G="$LARGE_FILE_DIR/file_32G.bin"

DATASET_DIR="$DATA_DIR/datasets"
SMALL_FILE_DIR="$DATASET_DIR/small_files_text"
TARBALL_PATH="$DATASET_DIR/large_tarball.tar"

CONDA_DIR="$DATA_DIR/conda_envs"
CONDA_ENV_SRC="/tmp/my-test-env" # Local scratch path
CONDA_ENV_DEST="$CONDA_DIR/test_env_node_A"

# --- Script Start ---
echo "Checking Data Plane at $DATA_DIR..."
mkdir -p "$LARGE_FILE_DIR"
mkdir -p "$DATASET_DIR"
mkdir -p "$CONDA_DIR"

# --- 1. Large Binaries (for Req 5) ---
echo "Checking for Large Binaries..."
if [ ! -f "$LARGE_FILE_32G" ]; then
    echo "  -> Generating 32GB file (file_32G.bin)..."
    dd if=/dev/urandom of="$LARGE_FILE_32G" bs=1G count=32 status=progress
else
    echo "  -> file_32G.bin already exists. Skipping."
fi

# --- 2. Small/Medium Files (for Req 1, 2) ---
echo "Checking for Small File Dataset..."
# We check if the directory is missing OR if it's empty
if [ ! -d "$SMALL_FILE_DIR" ] || [ -z "$(ls -A "$SMALL_FILE_DIR" 2>/dev/null)" ]; then
    echo "  -> Generating 100k small text files in small_files_text/..."
    mkdir -p "$SMALL_FILE_DIR" # Ensure it exists
    for i in $(seq 1 100000); do
        head -c 4K /dev/urandom > "$SMALL_FILE_DIR/file_$i.txt"
    done
else
    echo "  -> small_files_text/ directory already exists and is not empty. Skipping."
fi

# --- 3. Large Tarball (for Req 2) ---
echo "Checking for Large Tarball..."
if [ ! -f "$TARBALL_PATH" ]; then
    echo "  -> Creating large tarball (large_tarball.tar) from small files..."
    # Ensure source directory is not empty before tarring
    if [ -z "$(ls -A "$SMALL_FILE_DIR" 2>/dev/null)" ]; then
        echo "  -> ERROR: Cannot create tarball, source directory $SMALL_FILE_DIR is empty."
        exit 1
    fi
    tar -cf "$TARBALL_PATH" -C "$SMALL_FILE_DIR" .
else
    echo "  -> large_tarball.tar already exists. Skipping."
fi

# --- 4. Conda Environment (for Req 3) ---
echo "Checking for Conda Environment..."

# First, check if the *local source* env exists
if [ ! -d "$CONDA_ENV_SRC" ]; then
    echo "  -> Creating local conda env at $CONDA_ENV_SRC..."
    # This assumes 'conda' is in the PATH
    conda create -p "$CONDA_ENV_SRC" -y python=3.9 numpy pandas pytorch -c pytorch -c conda-forge
else
    echo "  -> Local conda env at $CONDA_ENV_SRC already exists."
fi

# Second, check if the *destination* env exists on the target FS
if [ ! -d "$CONDA_ENV_DEST" ]; then
    echo "  -> Copying Conda environment to $CONDA_ENV_DEST..."
    # Use archive mode (-a) to preserve links and permissions
    cp -a "$CONDA_ENV_SRC" "$CONDA_ENV_DEST"
else
    echo "  -> $CONDA_ENV_DEST already exists. Skipping."
fi

echo "Data Plane setup check complete."
