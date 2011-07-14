cmake_minimum_required(VERSION 2.8.5)
project(PROTO-RPC)

# Following needs to be in ~/.bashrc to get linux to pick up clang
# CC=/opt/root/bin/clang
# CXX=/opt/root/bin/clang++
# export CC
# export CXX

# PROTBUF #####################################################################
set( CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/dependencies/protobuf)

find_package(Protobuf REQUIRED)

if(PROTOBUF_FOUND)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
endif()
###############################################################################

# BOOST ######################################################################
set(Boost_ADDITIONAL_VERSIONS "1.47.0")

set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/dependencies/boost)

find_package(Boost  REQUIRED COMPONENTS system filesystem)
if(Boost_FOUND)
  include_directories(Boost_INCLUDE_DIRS)
  set(Boost_USE_MULTITHREADED      ON)
endif()
###############################################################################

include_directories(include)
# GENERATED CODE GOES INTO /src , therefore the generated headers need to be 
# picked up from here
include_directories("${CMAKE_BINARY_DIR}/src")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

add_subdirectory(src)
add_subdirectory(tests)
