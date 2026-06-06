cmake_minimum_required(VERSION 3.22)
project(snippets_cross_host_communication_c LANGUAGES C)

find_package(iceoryx2-c 0.9.999 REQUIRED)

add_executable(larry_telemetry src/larry_telemetry.c)
target_link_libraries(larry_telemetry iceoryx2-c::static-lib)

add_executable(dashboard src/dashboard.c)
target_link_libraries(dashboard iceoryx2-c::static-lib)
