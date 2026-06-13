cmake_minimum_required(VERSION 3.22)
project(snippets_event_c LANGUAGES C)

find_package(iceoryx2-c 0.9.999 REQUIRED)

add_executable(notifier src/notifier.c)
target_link_libraries(notifier iceoryx2-c::static-lib)

add_executable(listener src/listener.c)
target_link_libraries(listener iceoryx2-c::static-lib)
