add_executable(client.exe main.cpp)

find_package(absl CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

include_directories("../../protos")

target_link_libraries(client.exe
    PRIVATE
        hpcfs_bench_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        gRPC::grpc++          # Explicitly link
        gRPC::grpc            # Explicitly link
        gRPC::gpr             # Explicitly link
        # ADD ABSEIL LIBRARIES HERE:
        absl::base
        absl::log
        absl::strings
        absl::synchronization
        absl::time
        absl::flags
        # ... and any other Abseil components Protobuf/gRPC might pull in,
        # based on the undefined references. The list can be quite long.
        # Common ones include:
        absl::cord
        absl::memory
        absl::random_random
        absl::status
        absl::str_format
        # absl::container
        absl::hash
        absl::optional
        absl::span
        absl::utility

        absl::flags_parse
        absl::absl_log
        absl::log_initialize
        absl::log_globals
        # The specific list depends on the exact version and build of Protobuf/gRPC
        # and what Abseil features they use. Start with a core set and add more
        # as the linker errors indicate missing symbols.
        # A good starting point often includes:
        # absl::base absl::log absl::strings absl::time absl::synchronization
        # absl::random_random absl::status absl::flags absl::cord
)

#
## For gRPC headers (like grpc/grpc.h, grpcpp/grpcpp.h)
## These are provided by the gRPC::grpc++ imported target.
target_include_directories(server.exe
        PUBLIC
        $<TARGET_PROPERTY:gRPC::grpc++,INTERFACE_INCLUDE_DIRECTORIES> # Public headers for gRPC
        $<TARGET_PROPERTY:absl::base,INTERFACE_INCLUDE_DIRECTORIES> # Example Abseil component
        $<TARGET_PROPERTY:absl::strings,INTERFACE_INCLUDE_DIRECTORIES>
)