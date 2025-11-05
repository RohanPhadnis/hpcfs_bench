#include "../test_common.hpp"

class PosixPermissionsTest: public BaseTest {
private:
    std::string root;
    std::filesystem::path g_test_dir;
public:
    PosixPermissionsTest(const std::string& root): root(root) {}

    bool global_setup(std::vector<TestContext>& worker_contexts) {
        std::filesystem::create_directory(g_test_dir);
        std::string file_path = (g_test_dir / "perms.txt").string();
        
        // NOTE: This test requires worker 0 and 1 to run as different users!
        worker_contexts.clear();
        worker_contexts.push_back({0, 2, "owner", {{"file_path", file_path}}});
        worker_contexts.push_back({1, 2, "other", {{"file_path", file_path}}});
        return true;
    }
    void global_cleanup() {
        std::filesystem::remove_all(g_test_dir);
    }
    std::vector<TestResult> global_execute(
        // GrpcClientManager& grpc_clients,
        const std::vector<TestContext>& worker_contexts
    ) {
        std::vector<TestResult> results;
        
        // This is a 4-step sequential orchestration
        TestContext owner_ctx = worker_contexts[0];
        TestContext other_ctx = worker_contexts[1];

        // Step 1: Owner creates file 0600
        owner_ctx.params["step"] = "create_600";
        results.push_back(worker_execute(owner_ctx));// grpc_clients.rpc_call_execute(0, owner_ctx));
        if (!results.back().success) return results;

        // Step 2: Other user tries to read (should fail)
        other_ctx.params["step"] = "check_fail_read";
        results.push_back(worker_execute(other_ctx));//grpc_clients.rpc_call_execute(1, other_ctx));
        if (!results.back().success) return results;

        // Step 3: Owner changes perms to 0644
        owner_ctx.params["step"] = "chmod_644";
        results.push_back(worker_execute(owner_ctx));//grpc_clients.rpc_call_execute(0, owner_ctx));
        if (!results.back().success) return results;

        // Step 4: Other user tries to read (should pass)
        other_ctx.params["step"] = "check_pass_read";
        results.push_back(worker_execute(other_ctx));//grpc_clients.rpc_call_execute(1, other_ctx));
        
        return results;
    }

    bool worker_setup(const TestContext& context) { return true; }
    void worker_cleanup(const TestContext& context) {}
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        ScopedTimer timer(result.duration_ns);
        const auto& params = context.params;

        if (context.role == "owner_create") {
            int fd = open(params.at("file_path").c_str(), O_CREAT | O_WRONLY, 0600);
            TEST_ASSERT(fd >= 0, "owner_create: open(0600) failed", result);
            close(fd);
            result.success = true;
        } else if (context.role == "other_check_fail") {
            int fd = open(params.at("file_path").c_str(), O_RDONLY);
            bool check = (fd < 0 && errno == EACCES);
            TEST_ASSERT(check, "other_check_fail: open() did not fail with EACCES", result);
            result.success = true;
        } else if (context.role == "owner_chmod") {
            TEST_ASSERT(chmod(params.at("file_path").c_str(), 0644) == 0, "owner_chmod: chmod(0644) failed", result);
            result.success = true;
        } else if (context.role == "other_check_pass") {
            int fd = open(params.at("file_path").c_str(), O_RDONLY);
            TEST_ASSERT(fd >= 0, "other_check_pass: open(0644) failed to open", result);
            close(fd);
            result.success = true;
        }
        return result;
    }
};
