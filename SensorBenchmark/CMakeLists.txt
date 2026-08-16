cmake_minimum_required(VERSION 3.10)

project(
    SensorBenchmark
    VERSION 1.0.0
    DESCRIPTION "Dynamic Range Query data structure benchmark"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SENSOR_BENCHMARK_SOURCES
    src/main.c
    src/raw_array.c
    src/prefix_sum.c
    src/fenwick_tree.c
    src/segment_tree.c
    src/workload.c
    src/benchmark.c
    src/test.c
    src/demo.c
)

add_executable(
    sensor_benchmark
    ${SENSOR_BENCHMARK_SOURCES}
)

target_include_directories(
    sensor_benchmark
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(
        sensor_benchmark
        PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(
        sensor_benchmark
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        sensor_benchmark
        PRIVATE
        $<$<CONFIG:Release>:-O3>
    )
endif()

set_target_properties(
    sensor_benchmark
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}
)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/results
)

enable_testing()

add_test(
    NAME sensor_benchmark_tests
    COMMAND sensor_benchmark
)

set_tests_properties(
    sensor_benchmark_tests
    PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)