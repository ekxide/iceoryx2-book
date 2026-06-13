cmake_minimum_required(VERSION 3.22)
project(snippets_blackboard_c LANGUAGES C)

find_package(iceoryx2-c 0.9.999 REQUIRED)

add_executable(writer src/writer.c)
target_link_libraries(writer iceoryx2-c::static-lib)

add_executable(reader src/reader.c)
target_link_libraries(reader iceoryx2-c::static-lib)
