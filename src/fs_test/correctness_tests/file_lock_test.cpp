#include "../test_common.hpp"

class FileLockTest: public BaseTest {
private:
    int locking_test_fd;
    std::filesystem::path g_test_dir;
public:

    bool global_setup(std::vector<TestContext>& worker_contexts) {
        std::filesystem::create_directory(g_test_dir);
        std::string lock_file = (g_test_dir / "lock.file").string();
        creat(lock_file.c_str(), 0644); close(creat(lock_file.c_str(), 0644));

        worker_contexts.clear();
        worker_contexts.push_back({0, 2, "locker", {{"lock_file", lock_file}}});
        worker_contexts.push_back({1, 2, "try_locker", {{"lock_file", lock_file}}});
        return true;
    }
    void global_cleanup() {
        std::filesystem::remove_all(g_test_dir);
    }
    std::vector<TestResult> global_execute(
        // GrpcClientManager& grpc_clients,
        const std::vector<TestContext>& worker_contexts
    ) {
        // This test MUST be parallel.
        std::vector<TestResult> results(2);

        // 1. Call setup on both workers
        grpc_clients.rpc_call_setup(0, worker_contexts[0]);
        grpc_clients.rpc_call_setup(1, worker_contexts[1]);

        // 2. Launch execute calls in parallel
        auto fut_locker = std::async(std::launch::async, [&]() {
            return grpc_clients.rpc_call_execute(0, worker_contexts[0]);
        });
        auto fut_try_locker = std::async(std::launch::async, [&]() {
            return grpc_clients.rpc_call_execute(1, worker_contexts[1]);
        });

        results[0] = fut_locker.get();
        results[1] = fut_try_locker.get();

        // 3. Call cleanup on both workers
        grpc_clients.rpc_call_cleanup(0, worker_contexts[0]);
        grpc_clients.rpc_call_cleanup(1, worker_contexts[1]);
        
        return results;
    }

    bool worker_setup(const TestContext& context) {
        locking_test_fd = open(context.params.at("lock_file").c_str(), O_RDWR);
        return (locking_test_fd >= 0);
    }
    void worker_cleanup(const TestContext& context) {
        if (locking_test_fd >= 0) close(locking_test_fd);
        locking_test_fd = -1;
    }
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        ScopedTimer timer(result.duration_ns);
        TEST_ASSERT(locking_test_fd >= 0, "File not open (setup failed?)", result);

        if (context.role == "locker") {
            TEST_ASSERT(flock(locking_test_fd, LOCK_EX) == 0, "locker: flock(LOCK_EX) failed", result);
            std::this_thread::sleep_for(std::chrono::seconds(5));
            TEST_ASSERT(flock(locking_test_fd, LOCK_UN) == 0, "locker: flock(LOCK_UN) failed", result);
            result.success = true;
        } else if (context.role == "try_locker") {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Let locker get the lock
            bool check1 = (flock(locking_test_fd, LOCK_EX | LOCK_NB) != 0 && errno == EWOULDBLOCK);
            TEST_ASSERT(check1, "try_locker: Non-blocking lock did not fail as expected", result);
            
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait for release
            
            bool check2 = (flock(locking_test_fd, LOCK_EX) == 0);
            TEST_ASSERT(check2, "try_locker: Blocking lock failed after release", result);
            
            flock(locking_test_fd, LOCK_UN);
            result.success = true;
        }
        return result;
    }
};
