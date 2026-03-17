cmake_minimum_required(VERSION 4.10)

project(tensile)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_Declare(
  benchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.5.zip
)

FetchContent_MakeAvailable(benchmark)
FetchContent_MakeAvailable(googletest)

add_library(Tensile INTERFACE)

enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)



