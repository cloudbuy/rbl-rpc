ADD_EXECUTABLE(test_server1 test_server1.cc)
target_link_libraries(test_server1 ${Boost_LIBRARIES} marshall_rpc_proto ${PROTOBUF_LIBRARY} )
