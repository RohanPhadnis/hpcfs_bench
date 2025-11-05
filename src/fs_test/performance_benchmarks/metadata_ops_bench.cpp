#include "../test_common.hpp"

class MetadataOpsBench: public BaseTest {
public:
    bool worker_setup(const TestContext& context) { return true; }
    void worker_cleanup(const TestContext& context) {}
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        const auto& params = context.params;
        int num_threads = std::stoi(params.at("num_threads"));
        int files_per_worker = std::stoi(params.at("files_per_worker"));
        int files_per_thread = files_per_worker / num_threads;
        std::string test_dir = params.at("test_dir");
        int worker_id = context.worker_id;

        auto create_files_task = [=](int thread_id) {
            for (int i = 0; i < files_per_thread; ++i) {
                std::string file_path = test_dir + "/file_" + std::to_string(worker_id)
                                    + "_" + std::to_string(thread_id) + "_" + std::to_string(i);
                int fd = open(file_path.c_str(), O_CREAT | O_WRONLY, 0644);
                if (fd < 0) return false;
                close(fd);
            }
            return true;
        };

        std::vector<std::thread> threads;
        std::vector<bool> thread_results(num_threads);
        
        ScopedTimer timer(result.duration_ns);
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&, i]() {
                thread_results[i] = create_files_task(i);
            });
        }
        for (auto& t : threads) t.join();
        // Timer stops here

        for (bool res : thread_results) PERF_TEST_ASSERT(res, "A thread failed to create files", result);
        
        result.success = true;
        double duration_s = result.duration_ns / 1.0e9;
        double iops = static_cast<double>(files_per_worker) / duration_s;
        result.metrics["local_iops"] = std::to_string(iops);
        return result;
    }
};
