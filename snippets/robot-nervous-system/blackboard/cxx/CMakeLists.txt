cmake_minimum_required(VERSION 3.22)
project(snippets_blackboard_cxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(iceoryx2-cxx 0.9.999 REQUIRED)

add_executable(writer src/writer.cpp)
target_link_libraries(writer iceoryx2-cxx::static-lib-cxx)

add_executable(reader src/reader.cpp)
target_link_libraries(reader iceoryx2-cxx::static-lib-cxx)
