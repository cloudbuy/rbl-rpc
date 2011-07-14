add_executable(protoc-gen-RBLRPC proto_rbl_rpc_generator.cc)
set_target_properties(protoc-gen-RBLRPC PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools")
target_link_libraries(protoc-gen-RBLRPC ${PROTOBUF_PROTOC_LIBRARIES} ${Boost_LIBRARIES})
