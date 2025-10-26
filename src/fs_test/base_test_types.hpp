#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

/**
 * @brief The raw data container returned by a single worker after executing a test.
 *
 * This struct is "dumb" and only holds data. The server is responsible
 * for all interpretation, including aggregation and boolean pass/fail logic.
 */
struct TestResult {
    /**
     * @brief Set to false if this worker encountered a critical error
     * (e.g., open() failed, assertion failed).
     */
    bool success = true;

    /**
     * @brief A detailed error message if success == false.
     */
    std::string error_msg;

    /**
     * @brief Total wall-clock time for the worker_execute() method.
     */
    uint64_t duration_ns = 0;

    /**
     * @brief A flexible map for any test-specific raw data.
     *
     * Examples:
     * - {"inode": "12345"}
     * - {"throughput_gbps": "3.14"}
     * - {"cold_read_s": "10.5", "warm_read_s": "2.1"}
     */
    std::map<std::string, std::string> metrics;
};

/**
 * @brief The "work order" sent from the server (global_setup) to each worker.
 *
 * This struct contains all instructions a worker needs to perform
 * its specific role in the test.
 */
struct TestContext {
    /** @brief This worker's unique ID (e.g., 0, 1, 2...). */
    int worker_id;

    /** @brief The total number of workers participating in this test. */
    int total_workers;

    /**
     * @brief The specific role for this worker.
     * Examples: "default", "writer", "reader", "linker", "checker", "idle".
     */
    std::string role = "default";

    /**
     * @brief A map of custom parameters for this worker.
     * Examples: {"file_path": "/mnt/hpc-fs/file_0.bin"}, {"file_size_gb": "32"}
     */
    std::map<std::string, std::string> params;
};
