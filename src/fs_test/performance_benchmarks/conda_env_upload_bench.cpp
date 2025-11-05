#include "../test_common.hpp"

class CondaEnvUploadBench: public BaseTest {
public:
    bool worker_setup(const TestContext& context) {
        const auto& params = context.params;
        std::string local_path = params.at("local_conda_path");
        struct stat st;
        if (stat(local_path.c_str(), &st) != 0) {
            // Only create if it doesn't exist. This is idempotent.
            std::string cmd = "conda create -p " + local_path + " -y python=3.9 numpy";
            if (system(cmd.c_str()) != 0) return false;
        }
        return true;
    }
    void worker_cleanup(const TestContext& context) {
        // Note: We might want to *keep* the local env to speed up setup next time.
        // system(("rm -rf " + context.params.at("local_conda_path")).c_str());
    }
    TestResult worker_execute(const TestContext& context) {
        TestResult result;
        const auto& params = context.params;
        std::string local_path = params.at("local_conda_path");
        std::string target_path_cp = params.at("target_path_cp");
        std::string target_path_tar = params.at("target_path_tar");

        // 1. Time `cp -a`
        std::string cmd_cp = "cp -a " + local_path + " " + target_path_cp;
        uint64_t time_cp_ns;
        {
            ScopedTimer timer(time_cp_ns);
            system(cmd_cp.c_str());
        }
        result.metrics["time_cp_a_s"] = std::to_string(time_cp_ns / 1.0e9);

        // 2. Time `tar`
        std::string cmd_tar = "tar -cf - -C " + local_path + " . | (cd " + target_path_tar + " && tar -xf -)";
        uint64_t time_tar_ns;
        {
            ScopedTimer timer(time_tar_ns);
            system(cmd_tar.c_str());
        }
        result.metrics["time_tar_s"] = std::to_string(time_tar_ns / 1.0e9);

        result.success = true;
        return result;
    }
};