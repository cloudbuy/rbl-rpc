add_library(rpc_server STATIC rpc_common.cc backend.cc ClientServiceCookies.cc)
target_link_libraries(rpc_server marshall_rpc_proto rpc_common)
