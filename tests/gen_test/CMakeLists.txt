ADD_EXECUTABLE(test_gen_code main.cc)
target_link_libraries(test_gen_code ${Boost_LIBRARIES} ${GTEST_LIBRARIES}  -lpthread)
