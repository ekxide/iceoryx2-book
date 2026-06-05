cmake_minimum_required(VERSION 3.22)
project(snippets_cross_host_communication_cxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(iceoryx2-cxx 0.9.999 REQUIRED)

add_executable(larry_telemetry src/larry_telemetry.cpp)
target_link_libraries(larry_telemetry iceoryx2-cxx::static-lib-cxx)

add_executable(dashboard src/dashboard.cpp)
target_link_libraries(dashboard iceoryx2-cxx::static-lib-cxx)
