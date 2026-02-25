cmake_minimum_required(VERSION 3.16)
project(pdv_threadpool LANGUAGES CXX)

# PDV požaduje C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Vlákna
find_package(Threads REQUIRED)

add_executable(pdv_threadpool
        src/main.cpp
        src/main.cpp
        src/main.cpp
        src/main.cpp
)

target_link_libraries(pdv_threadpool PRIVATE Threads::Threads)