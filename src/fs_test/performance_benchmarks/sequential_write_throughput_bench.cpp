#include "../test_common.hpp"

class SequentialWriteThroughputBench: public BaseTest {
private:
    std::vector<char> write_buffer;
    int write_fd;
public:
    bool worker_setup(const TestContext& context) override {
        size_t block_size_mb = std::stoll(context.params.at("block_size_mb"));
        write_buffer.resize(block_size_mb * 1024 * 1024);
        // Fill buffer once
        std::fill(write_buffer.begin(), write_buffer.end(), 'A');

        write_fd = open(context.params.at("file_path").c_str(), O_CREAT | O_WRONLY | O_DIRECT, 0644);
        return (write_fd >= 0);
    }
    void worker_cleanup(const TestContext& context) {
        if (write_fd >= 0) close(write_fd);
        write_fd = -1;
        write_buffer.clear();
    }
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        PERF_TEST_ASSERT(write_fd >= 0, "File not open (setup failed?)", result);

        long long size_gb = std::stoll(context.params.at("file_size_gb"));
        long long bytes_to_write = size_gb * 1024 * 1024 * 1024;
        long long bytes_written = 0;
        size_t block_size = write_buffer.size();

        ScopedTimer timer(result.duration_ns);
        while (bytes_written < bytes_to_write) {
            ssize_t written = write(write_fd, write_buffer.data(), block_size);
            PERF_TEST_ASSERT(written == (ssize_t)block_size, "write() failed", result);
            bytes_written += written;
        }
        fdatasync(write_fd); // Ensure data is on disk
        
        // Timer stops here
        result.success = true;
        double duration_s = result.duration_ns / 1.0e9;
        double gbps = static_cast<double>(size_gb) / duration_s;
        result.metrics["throughput_gbps"] = std::to_string(gbps);
        return result;
    }
};
