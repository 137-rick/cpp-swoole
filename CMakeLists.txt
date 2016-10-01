cmake_minimum_required(VERSION 2.4)
project(server)

INCLUDE_DIRECTORIES(BEFORE /usr/local/lib/)
INCLUDE_DIRECTORIES(BEFORE ./include /opt/swoole/include)
LINK_DIRECTORIES(/usr/local/lib)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -Wall")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

SET(SOURCE_FILES server.cpp
    src/Server.cpp
)

add_executable(server ${SOURCE_FILES})
target_link_libraries(server libswoole.so)