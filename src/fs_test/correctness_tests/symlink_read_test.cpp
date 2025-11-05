#include "../test_common.hpp"

class SymlinkReadTest: public BaseTest {
private:
    std::string root;
    std::filesystem::path g_test_dir;
public:
    SymlinkReadTest(const std::string& root): root(root) {
        g_test_dir = root + "/hardlink_test";
    }
    
    bool global_setup(std::vector<TestContext>& worker_contexts) {
        std::filesystem::create_directory(g_test_dir);
        std::string target = (g_test_dir / "sym_target.txt").string();
        std::string link = (g_test_dir / "sym_link.txt").string();
        std::string data = "TestData123";

        std::ofstream(target) << data; // Create and write data to target

        worker_contexts.clear();
        worker_contexts.push_back({0, 2, "linker", {{"target_path", target}, {"link_path", link}}});
        worker_contexts.push_back({1, 2, "reader", {{"link_path", link}, {"expected_data", data}}});
        return true;
    }
    void global_cleanup() {
        std::filesystem::remove_all(g_test_dir);
    }
    std::vector<TestResult> global_execute(
        // GrpcClientManager& grpc_clients,
        const std::vector<TestContext>& worker_contexts
    ) {
        // Sequential test
        TestResult linker_res = worker_execute(worker_contexts[0]);// grpc_clients.rpc_call_execute(0, worker_contexts[0]);
        if (!linker_res.success) return {linker_res};
        
        TestResult reader_res = worker_execute(worker_contexts[0]);//.rpc_call_execute(1, worker_contexts[1]);
        return {linker_res, reader_res};
    }

    bool worker_setup(const TestContext& context) { return true; }
    void worker_cleanup(const TestContext& context) {}
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        ScopedTimer timer(result.duration_ns);
        const auto& params = context.params;

        if (context.role == "linker") {
            TEST_ASSERT(symlink(params.at("target_path").c_str(), params.at("link_path").c_str()) == 0, "symlink() failed", result);
            result.success = true;
        } else if (context.role == "reader") {
            int fd = open(params.at("link_path").c_str(), O_RDONLY);
            TEST_ASSERT(fd >= 0, "open(link_path) failed", result);
            
            char buffer[128];
            ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
            close(fd);
            TEST_ASSERT(bytes_read > 0, "read() failed", result);
            
            buffer[bytes_read] = '\0';
            std::string data_read = buffer;
            result.metrics["data_read"] = data_read;
            TEST_ASSERT(data_read == params.at("expected_data"), "Data mismatch!", result);
            result.success = true;
        }
        return result;
    }
};