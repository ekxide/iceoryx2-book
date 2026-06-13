cmake_minimum_required(VERSION 3.22)
project(snippets_request_response_cxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(iceoryx2-cxx 0.9.999 REQUIRED)

add_executable(client src/client.cpp)
target_link_libraries(client iceoryx2-cxx::static-lib-cxx)

add_executable(server src/server.cpp)
target_link_libraries(server iceoryx2-cxx::static-lib-cxx)
