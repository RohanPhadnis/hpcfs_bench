#include <iostream>
#include <memory>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "../../protos/hpcfs_bench.pb.h"
#include "../../protos/hpcfs_bench.grpc.pb.h"


class ClusterServiceReactor : public grpc::ServerBidiReactor<hpcfs_bench::TestRequest, hpcfs_bench::TestResult> {
private:
    bool active;
public:
    ClusterServiceReactor(): active(true) {
        std::cout << "Reactor created" << std::endl;
    }
    void OnDone() override {
        std::cout << "Reactor done" << std::endl;
        active = false;
    }
};


class ClusterService : public hpcfs_bench::ClusterService::Service {
    ClusterService() {}
    grpc::ServerBidiReactor< hpcfs_bench::TestRequest, hpcfs_bench::TestResult>* Comm(grpc::CallbackServerContext* context) {
        ClusterServiceReactor* reactor = new ClusterServiceReactor();
        return reactor;
    }
};


void run_server() {
    std::string server_address("0.0.0.0:8000");
    ClusterService service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
}

int main() {
    run_server();

    return 0;
}