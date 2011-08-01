ADD_EXECUTABLE(test_service_oracle TestServiceOracle.cc)
target_link_libraries(test_service_oracle ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${PROTOBUF_LIBRARY} marshall_rpc_proto rpc_common rpc_server)
set_target_properties(test_service_oracle PROPERTIES COMPILE_DEFINITIONS ISOLATED_GTEST_COMPILE)

