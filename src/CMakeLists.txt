cmake_minimum_required (VERSION 2.6)
project (relay)
add_definitions(-DASIO_STANDALONE)
include_directories(../deps/asio/include)
include_directories(../deps/googletest/googletest/include)
include_directories(../deps/kcp)
link_libraries(-lgtestd -lpthread)
link_directories(../deps/googletest/googlemock/gtest/)
add_executable(test test.cc)
add_executable(client ../deps/kcp/ikcp.c client.cc)
add_executable(server ../deps/kcp/ikcp.c server.cc)
