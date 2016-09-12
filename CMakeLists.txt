cmake_minimum_required(VERSION 2.4)
project(server)

INCLUDE_DIRECTORIES(BEFORE /opt/swoole/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(SOURCE_FILES server.cpp)

add_executable(server ${SOURCE_FILES})
target_link_libraries(server /opt/swoole/lib/libswoole.so)
