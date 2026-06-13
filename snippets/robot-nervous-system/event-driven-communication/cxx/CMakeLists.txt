cmake_minimum_required(VERSION 3.22)
project(snippets_event_driven_communication_cxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(iceoryx2-cxx 0.9.999 REQUIRED)

add_executable(publisher src/publisher.cpp)
target_link_libraries(publisher iceoryx2-cxx::static-lib-cxx)

add_executable(subscriber src/subscriber.cpp)
target_link_libraries(subscriber iceoryx2-cxx::static-lib-cxx)
