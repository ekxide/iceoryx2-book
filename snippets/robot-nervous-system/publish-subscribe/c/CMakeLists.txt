cmake_minimum_required(VERSION 3.22)
project(snippets_publish_subscribe_c LANGUAGES C)

find_package(iceoryx2-c 0.9.999 REQUIRED)

add_executable(publisher src/publisher.c)
target_link_libraries(publisher iceoryx2-c::static-lib)

add_executable(subscriber src/subscriber.c)
target_link_libraries(subscriber iceoryx2-c::static-lib)
