cmake_minimum_required(VERSION 3.22.1)
project(hpcfs_bench VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(gRPC_DIR /root/.local/lib/cmake/grpc)

add_subdirectory(protos)
add_subdirectory(src)
