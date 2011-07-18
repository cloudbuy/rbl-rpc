add_executable(test_client_1 test_client1.cc)
target_link_libraries(test_client_1 ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY} generator_rpc_proto)
