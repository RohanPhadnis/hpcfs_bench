#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "../../protos/hpcfs_bench.pb.h"
#include "../../protos/hpcfs_bench.grpc.pb.h"


class ClusterClient : public grpc::ClientBidiReactor<hpcfs_bench::TestResult, hpcfs_bench::TestRequest> {
private:

    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<hpcfs_bench::ClusterService::Stub> stub;
    grpc::ClientContext context;

    hpcfs_bench::TestRequest request;
    hpcfs_bench::TestResult result;

    grpc::Status status;
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    
public:
    ClusterClient() {
        channel = std::shared_ptr<grpc::Channel>(grpc::CreateChannel("0.0.0.0:8000", grpc::InsecureChannelCredentials()));
        stub = hpcfs_bench::ClusterService::NewStub(channel);
        stub->async()->Comm(&context, this);
        StartWrite(&result);
        StartRead(&request);
        StartCall();
        std::cout << "Client created" << std::endl;
    }
    void perform_test(const hpcfs_bench::TestRequest& req, hpcfs_bench::TestResult& res) {
        std::cout << "Performing test: " << req.DebugString() << std::endl;
    }
    void OnReadDone(bool ok) override {
        if (ok) {
            // hpcfs_bench::TestResult result = GetReadMessage();
            std::cout << "Received TestResult: " << request.DebugString() << std::endl;

            // perform test
            perform_test(request, request);

            // send results back to server
            StartWrite(result);

            // Continue reading
            StartRead(&request);
        } else {
            std::cout << "No more TestResults from server." << std::endl;
        }
    }
    void OnWriteDone(bool ok) override {
        if (ok) {
            std::cout << "TestRequest sent successfully." << std::endl;
        } else {
            std::cout << "Failed to send TestRequest." << std::endl;
        }
    }
    void OnDone(const grpc::Status& status) override {
        std::unique_lock<std::mutex> l(mutex);
        this->status = status;
        done = true;
        cv.notify_all();
        std::cout << "Client done" << std::endl;
    }
    Status Await() {
        std::unique_lock<std::mutex> l(mutex);
        cv.wait(l, [this] { return done; });
        return std::move(status);
    }
};


int main() {
    ClusterClient client;
    client.Await();
    return 0;
}
