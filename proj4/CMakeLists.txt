cmake_minimum_required(VERSION 3.14)
project(proj4)

set(CMAKE_CXX_STANDARD 14)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
add_executable(proj4 proj4_server.cpp)
target_link_libraries(proj4 Threads::Threads)