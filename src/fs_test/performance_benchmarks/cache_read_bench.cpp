#include "../test_common.hpp"

class CacheReadBench: public BaseTest {
private:
    std::vector<char> read_buffer;
public:
    bool worker_setup(const TestContext& context) {
        read_buffer.resize(1 * 1024 * 1024); // 1MB read buffer
        return true;
    }
    void worker_cleanup(const TestContext& context) {
        read_buffer.clear();
    }
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        ScopedTimer timer(result.duration_ns); // Times the *whole* operation
        const auto& params = context.params;
        
        auto time_read = [&](const std::string& file_path) -> double {
            int fd = open(file_path.c_str(), O_RDONLY | O_DIRECT);
            if (fd < 0) return -1.0;
            
            auto start = std::chrono::high_resolution_clock::now();
            while (read(fd, read_buffer.data(), read_buffer.size()) > 0);
            auto end = std::chrono::high_resolution_clock::now();
            
            close(fd);
            return std::chrono::duration<double>(end - start).count();
        };

        std::string file_path = params.at("file_path");
        long long size_gb = std::stoll(params.at("file_size_gb"));

        // 1. Cold Read
        system("sudo echo 3 > /proc/sys/vm/drop_caches");
        double cold_s = time_read(file_path);
        PERF_TEST_ASSERT(cold_s > 0, "Cold read failed", result);

        // 2. Warm Read
        system("sudo echo 3 > /proc/sys/vm/drop_caches"); // Clear OS cache *again*
        double warm_s = time_read(file_path);
        PERF_TEST_ASSERT(warm_s > 0, "Warm read failed", result);

        result.success = true;
        result.metrics["cold_read_gbps"] = std::to_string(size_gb / cold_s);
        result.metrics["warm_read_gbps"] = std::to_string(size_gb / warm_s);
        return result;
    }
};