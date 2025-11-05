#pragma once

// --- C++ Standard Libs ---
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <filesystem> // For C++17 filesystem operations

// --- C POSIX Libs ---
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h> // for flock
#include <fcntl.h>    // for open, O_DIRECT
#include <unistd.h>   // for link, symlink, unlink, read, write, close, lseek
#include <errno.h>    // for errno
#include <string.h>   // for strerror
#include <cstdlib>    // for system()

// --- Project Headers ---
#include "base_test.hpp"
#include "base_test_types.hpp" // todo: replace with protobuf-defined types or implement a converter

// --- Helper Functions ---
#define TEST_ASSERT(cond, msg, result_obj) \
    if (!(cond)) { \
        result_obj.success = false; \
        result_obj.error_msg = (msg) + std::string(" (errno: ") + get_error_str() + ")"; \
        return result_obj; \
    }
#define PERF_TEST_ASSERT(cond, msg, result_obj) \
    if (!(cond)) { \
        result_obj.success = false; \
        result_obj.error_msg = (msg) + std::string(" (errno: ") + get_error_str() + ")"; \
        return result_obj; \
    }

/**
 * @brief Gets a cross-platform string representation of the last system error.
 */
inline std::string get_error_str() {
    return strerror(errno);
}


/**
 * @brief Simple RAII timer.
 * Usage:
 * {
 * ScopedTimer t(result.duration_ns);
 * // ... code to time ...
 * } // duration_ns is set on destruction
 */
class ScopedTimer {
public:
    explicit ScopedTimer(uint64_t& duration_ns_out)
        : start_time_(std::chrono::high_resolution_clock::now()),
          duration_out_(duration_ns_out) {}

    ~ScopedTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        duration_out_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time_
        ).count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
    uint64_t& duration_out_;
};

