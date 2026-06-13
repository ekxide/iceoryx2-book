cmake_minimum_required(VERSION 3.22)
project(snippets_event_cxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(iceoryx2-cxx 0.9.999 REQUIRED)

add_executable(notifier src/notifier.cpp)
target_link_libraries(notifier iceoryx2-cxx::static-lib-cxx)

add_executable(listener src/listener.cpp)
target_link_libraries(listener iceoryx2-cxx::static-lib-cxx)
