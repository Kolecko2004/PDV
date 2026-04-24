cmake_minimum_required(VERSION 3.15)
project(pdv_lab02)

set(CMAKE_CXX_STANDARD 20)

add_compile_options("-march=native")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Wno-unused-parameter")

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(02_1threads_benchmark
        src/1threads_benchmark.cpp
        src/1threads.cpp
)

target_link_libraries(02_1threads_benchmark
        PUBLIC
        Threads::Threads
        OpenMP::OpenMP_CXX
)

add_executable(02_2condition_variable
        src/2condition_variable.cpp
)

target_link_libraries(02_2condition_variable
        PUBLIC
        Threads::Threads
)