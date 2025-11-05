python3 -m grpc_tools.protoc \
    --python_out=. \
    --grpc_python_out=. \
    --proto_path=/Users/rohanphadnis/GeorgiaTech/ArtisanResearch/hpcfs_bench/protos \
    hpcfs_bench.proto
