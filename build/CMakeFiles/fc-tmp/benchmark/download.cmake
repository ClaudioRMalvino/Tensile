cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

message(VERBOSE "Executing download step for benchmark")

block(SCOPE_FOR VARIABLES)

include("/home/claudio/Projects/cpp/Tensile/build/CMakeFiles/fc-stamp/benchmark/download-benchmark.cmake")
include("/home/claudio/Projects/cpp/Tensile/build/CMakeFiles/fc-stamp/benchmark/verify-benchmark.cmake")
include("/home/claudio/Projects/cpp/Tensile/build/CMakeFiles/fc-stamp/benchmark/extract-benchmark.cmake")


endblock()
