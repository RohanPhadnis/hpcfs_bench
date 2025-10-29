execute_process(
        COMMAND
        bash # The executable
        generate.sh
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" # Best practice to quote paths
        RESULT_VARIABLE PROTO_EXEC_RESULT
        OUTPUT_VARIABLE PROTO_EXEC_OUTPUT
        ERROR_VARIABLE PROTO_EXEC_ERROR
)

# Optional: Print results for debugging
message(STATUS "protoc execution result: ${PROTO_EXEC_RESULT}")
message(STATUS "protoc stdout: ${PROTO_EXEC_OUTPUT}")
message(STATUS "protoc stderr: ${PROTO_EXEC_ERROR}")

set(grpc_src ${CMAKE_CURRENT_SOURCE_DIR}/hpcfs_bench.grpc.pb.cc)
set(grpc_hdr ${CMAKE_CURRENT_SOURCE_DIR}/hpcfs_bench.grpc.pb.h)
set(pb_src ${CMAKE_CURRENT_SOURCE_DIR}/hpcfs_bench.pb.cc)
set(pb_hdr ${CMAKE_CURRENT_SOURCE_DIR}/hpcfs_bench.pb.h)
set(grpc_DIR /root/.local/lib/cmake/grpc)

find_package(absl)
find_package(Protobuf)
find_package(grpc)

add_library(hpcfs_bench_grpc_proto
    ${grpc_src}
    ${grpc_hdr}
    ${pb_src}
    ${pb_hdr}
)
target_link_libraries(hpcfs_bench_grpc_proto
    absl::check
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
)
