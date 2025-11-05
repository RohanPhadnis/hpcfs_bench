#include "../test_common.hpp"

class HardlinkTest: public BaseTest {
private:
    std::string root;
    std::filesystem::path g_test_dir;
public:
    HardlinkTest(const std::string& root): root(root) {
        g_test_dir = root + "/hardlink_test";
    }
    bool global_setup(std::vector<TestContext>& worker_contexts) {
        std::filesystem::create_directory(g_test_dir);
        std::string file_a = (g_test_dir / "hardlink_A").string();
        std::string file_b = (g_test_dir / "hardlink_B").string();

        // Create the initial file
        int fd = creat(file_a.c_str(), 0644);
        if (fd < 0) return false;
        close(fd);

        worker_contexts.clear();
        worker_contexts.push_back({0, 2, "linker",  {{"file_a", file_a}, {"file_b", file_b}}});
        worker_contexts.push_back({1, 2, "checker", {{"file_b", file_b}}});
        // Assume idle for others...
        return true;
    }
    void global_cleanup() {
        std::filesystem::remove_all(g_test_dir);
    }
    std::vector<TestResult> global_execute(
        // GrpcClientManager& grpc_clients,
        const std::vector<TestContext>& worker_contexts
    ) {
        std::vector<TestResult> results(worker_contexts.size());
        
        // This test is sequential. The linker MUST run before the checker.
        TestResult linker_res = worker_execute(worker_contexts[0]);// grpc_clients.rpc_call_execute(0, worker_contexts[0]);
        results[0] = linker_res;
        
        if (!linker_res.success) return results; // Stop if link failed

        // Pass the inode from the linker to the checker for validation
        TestContext checker_context = worker_contexts[1];
        checker_context.params["expected_inode"] = linker_res.metrics.at("inode_a");

        TestResult checker_res = worker_execute(worker_contexts[1]); //grpc_clients.rpc_call_execute(1, checker_context);
        results[1] = checker_res;
        
        return results;
    }

    bool worker_setup(const TestContext& context) { return true; }
    void worker_cleanup(const TestContext& context) {}
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        ScopedTimer timer(result.duration_ns);
        const auto& params = context.params;

        if (context.role == "linker") {
            struct stat st;
            TEST_ASSERT(stat(params.at("file_a").c_str(), &st) == 0, "stat(file_a) failed", result);
            ino_t inode_a = st.st_ino;
            TEST_ASSERT(link(params.at("file_a").c_str(), params.at("file_b").c_str()) == 0, "link() failed", result);
            result.metrics["inode_a"] = std::to_string(inode_a);
            result.success = true;
        } else if (context.role == "checker") {
            struct stat st;
            TEST_ASSERT(stat(params.at("file_b").c_str(), &st) == 0, "stat(file_b) failed", result);
            ino_t inode_b = st.st_ino;
            result.metrics["inode_b"] = std::to_string(inode_b);
            result.success = true;
        } else if (context.role == "idle") {
            result.success = true;
        }
        return result;
    }
};
