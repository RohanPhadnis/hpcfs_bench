#include <iostream>

#include <grpc/grpc.h>

#include "../../protos/hpcfs_bench.pb.h"
#include "../../protos/hpcfs_bench.grpc.pb.h"


class ClusterClient : public grpc::ClientBidiReactor<hpcfs_bench::TestRequest, hpcfs_bench::TestResult> {
private:
    grpc::channel channel;
    hpcfs_bench::ClusterClient::Stub stub;
public:
    ClusterClient() {
        channel = grpc::CreateChannel("0.0.0.0:8000", grpc::InsecureChannelCredentials());
        stub->async()->RouteChat(&context_, this);
        StartCall(&channel);
        std::cout << "Client created" << std::endl;
    }
    void OnDone() override {
        std::cout << "Client done" << std::endl;
    }
    void OnReadDone(bool ok) override {
        if (ok) {
            hpcfs_bench::TestResult result = GetReadMessage();
            std::cout << "Received TestResult: " << result.DebugString() << std::endl;
            // Continue reading
            StartRead();
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
};


int main() {
    ClusterClient client;
    Status status = client.Await();
    return 0;
}
