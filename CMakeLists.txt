cmake_minimum_required(VERSION 3.16)
project(compaction_optimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(compaction_optimizer
    src/simulator.cpp
)
target_include_directories(compaction_optimizer PUBLIC include)
target_compile_options(compaction_optimizer PRIVATE -Wall -Wextra -Wpedantic)

add_executable(compaction_benchmark benchmark/benchmark.cpp)
target_link_libraries(compaction_benchmark PRIVATE compaction_optimizer)

enable_testing()
add_executable(compaction_tests tests/simulator_test.cpp)
target_link_libraries(compaction_tests PRIVATE compaction_optimizer)
add_test(NAME compaction_tests COMMAND compaction_tests)

