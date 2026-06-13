cmake_minimum_required(VERSION 3.22)
project(snippets_request_response_c LANGUAGES C)

find_package(iceoryx2-c 0.9.999 REQUIRED)

add_executable(client src/client.c)
target_link_libraries(client iceoryx2-c::static-lib)

add_executable(server src/server.c)
target_link_libraries(server iceoryx2-c::static-lib)
