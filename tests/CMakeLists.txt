# add_subdirectory(basic_client_compilation_tests)
add_subdirectory(rpc_tests)

set(MARSHALL_DATA_DIR \"${CMAKE_CURRENT_SOURCE_DIR}/data/marshall\")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/marshall)

configure_file( "data/marshall.info.in" "${CMAKE_CURRENT_SOURCE_DIR}/data/marshall.info")
