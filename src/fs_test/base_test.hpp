#pragma once

#include "base_test_types.hpp"
#include <vector>
#include <map>

// --- Forward Declaration ---
// Forward-declare your gRPC stub container to avoid including gRPC headers
// in your main test interface.
namespace your_project {
    class GrpcClientManager; // Or whatever your stub container is
}

/**
 * @brief The abstract base class for all distributed filesystem tests.
 *
 * This interface separates the "Controller" logic (global_*) from the
 * "Agent" logic (worker_*). The server instantiates a test, calls its
 * global methods, which in turn invoke the worker methods on clients via gRPC.
 */
class BaseTest {
public:
    virtual ~BaseTest() = default;

    // --- 1. Server-Side (Controller) Methods ---

    /**
     * @brief Runs ONCE on the Server before any worker action.
     *
     * Its job is to perform any global setup (like creating a test directory)
     * and to define the work for each client by populating the worker_contexts.
     *
     * @param worker_contexts (out) An empty vector to be populated with
     * TestContext structs, one for each worker.
     * @return true if setup was successful, false otherwise.
     */
    virtual bool global_setup(std::vector<TestContext>& worker_contexts) = 0;

    /**
     * @brief Runs ONCE on the Server. This is the "main" function of the test.
     *
     * It implements the test's coordination logic. It is responsible for:
     * 1. Broadcasting worker_setup() to clients.
     * 2. Broadcasting worker_execute() to clients (sequentially or in parallel).
     * 3. Waiting for and collecting all TestResult structs.
     * 4. Broadcasting worker_cleanup() to clients.
     *
     * @param grpc_clients A manager/list of gRPC clients to communicate with workers.
     * @param worker_contexts (in) The work orders created by global_setup().
     * @return A vector of raw TestResult structs from all workers.
     */
    virtual std::vector<TestResult> global_execute(
        your_project::GrpcClientManager& grpc_clients,
        const std::vector<TestContext>& worker_contexts
    ) = 0;

    /**
     * @brief Runs ONCE on the Server *after* all workers have cleaned up.
     *
     * Its job is to clean up any global resources (like the test directory).
     */
    virtual void global_cleanup() = 0;


    // --- 2. Worker-Side (Agent) Methods ---
    // These methods will be called by the server via gRPC endpoints.

    /**
     * @brief Runs ONCE on EACH worker *before* worker_execute.
     *
     * Use this for any local setup, like pre-allocating memory buffers.
     *
     * @param context (in) This worker's specific instructions.
     * @return true on success, false on failure.
     */
    virtual bool worker_setup(const TestContext& context) = 0;

    /**
     * @brief Runs ONCE on EACH worker. This is the core test logic.
     *
     * @param context (in) This worker's specific instructions.
     * @return The raw TestResult for this worker.
     */
    virtual TestResult worker_execute(const TestContext& context) = 0;

    /**
     * @brief Runs ONCE on EACH worker *after* worker_execute.
     *
     * Use this to clean up any local resources (e.g., free buffers, close files).
     */
    virtual void worker_cleanup(const TestContext& context) = 0;
};
