#include <iostream>
#include <memory>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "../comm_utils/communicator.hpp"

#include "../../protos/hpcfs_bench.pb.h"
#include "../../protos/hpcfs_bench.grpc.pb.h"



class ServerCommunicator {
private:
    std::vector<std::shared_ptr<Communicator<hpcfs_bench::TestParams, hpcfs_bench::TestResult>>> communicators;
public:
    ServerCommunicator() {}
    std::shared_ptr<Communicator<hpcfs_bench::TestParams, hpcfs_bench::TestResult>> create_communicator() {
        auto comm = std::make_shared<Communicator<hpcfs_bench::TestParams, hpcfs_bench::TestResult>>();
        communicators.push_back(comm);
        return comm;
    }
    void send_to(size_t index, hpcfs_bench::TestParams&& params) {
        communicators[index]->queue_send(params);
    }
    hpcfs_bench::TestResult&& receive_from(size_t index) {
        return communicators[index]->receive();
    }
};


class ClusterServiceReactor : public grpc::ServerBidiReactor<hpcfs_bench::TestResult, hpcfs_bench::TestParams> {
private:
    bool done = false;

    hpcfs_bench::TestParams params;
    hpcfs_bench::TestResult result;
    
    std::shared_ptr<Communicator<hpcfs_bench::TestParams, hpcfs_bench::TestResult>> communicator;

public:
    ClusterServiceReactor(const std::shared_ptr<Communicator<hpcfs_bench::TestParams, hpcfs_bench::TestResult>>& communicator): communicator(communicator) {
        std::cout << "Reactor created" << std::endl;
        params = communicator->send();
        StartWrite(&params);
    }
    void OnReadDone(bool ok) override {
        if (ok) {
            // put the results in the receive queue
            communicator->queue_receive(result);

            // get next params to send
            params = communicator->send();
            // send params
            StartWrite(&params);
        } else {
            std::cout << "No more TestParams from client." << std::endl;
        }
    }
    void OnWriteDone(bool ok) override {
        if (ok) {
            // read next result
            StartRead(&result);
        } else {
            std::cout << "Failed to send TestParams." << std::endl;
        }
    }
    void OnDone() override {
        std::cout << "Reactor done" << std::endl;
        done = true;
    }
};


class ClusterService : public hpcfs_bench::ClusterService::Service {
private:
    std::shared_ptr<ServerCommunicator> server_communicator;
public:
    ClusterService(const std::shared_ptr<ServerCommunicator>& server_communicator): server_communicator(server_communicator) {}
    grpc::ServerBidiReactor< hpcfs_bench::TestResult, hpcfs_bench::TestParams>* Comm(grpc::CallbackServerContext* context) {
        ClusterServiceReactor* reactor = new ClusterServiceReactor(server_communicator->create_communicator());
        return reactor;
    }
};


class ControllerReactor : public grpc::ServerUnaryReactor<hpcfs_bench::TestParams, hpcfs_bench::TestBatchResult> {

}


class ControllerService : public hpcfs_bench::ControllerService::Service {
private:
    std::shared_ptr<ServerCommunicator> server_communicator;
public:
    ControllerService(const std::shared_ptr<ServerCommunicator>& server_communicator): server_communicator(server_communicator) {}
    grpc::ServerUnaryReactor* RunTests(grpc::ServerContext* context, const hpcfs_bench::TestParams* request, hpcfs_bench::TestBatchResult* response) override {
        return new ControllerReactor();
    }
};

void run_server() {
    std::string server_address("0.0.0.0:8000");
    std::shared_ptr<ServerCommunicator> server_communicator = std::make_shared<ServerCommunicator>();
    
    ClusterService service(server_communicator);
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
