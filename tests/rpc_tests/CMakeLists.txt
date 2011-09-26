ADD_EXECUTABLE(test_service_oracle TestServiceOracle.cc)
target_link_libraries(test_service_oracle ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${PROTOBUF_LIBRARY} marshall_rpc_proto rpc_common rpc_server)
set_target_properties(test_service_oracle PROPERTIES COMPILE_DEFINITIONS ISOLATED_GTEST_COMPILE)

ADD_EXECUTABLE(test_service_client_factory TestServiceClientFactory.cc)
target_link_libraries( test_service_client_factory ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${PROTOBUF_LIBRARY} marshall_rpc_proto rpc_common rpc_server rpc_client)
set_target_properties(test_service_client_factory PROPERTIES COMPILE_DEFINITIONS ISOLATED_GTEST_COMPILE)


ADD_EXECUTABLE(test_TcpRpc testTcpRpc.cc)
target_link_libraries(test_TcpRpc ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${PROTOBUF_LIBRARY} marshall_rpc_proto rpc_common rpc_server rpc_client)
set_target_properties(test_TcpRpc PROPERTIES COMPILE_DEFINITIONS ISOLATED_GTEST_COMPILE)

ADD_EXECUTABLE(test_RpcExceptions TestRpcExceptions.cc)
target_link_libraries(test_RpcExceptions ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${PROTOBUF_LIBRARY} marshall_rpc_proto rpc_common rpc_server rpc_client)
set_target_properties(test_RpcExceptions PROPERTIES COMPILE_DEFINITIONS ISOLATED_GTEST_COMPILE)

