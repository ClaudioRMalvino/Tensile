cmake_minimum_required(VERSION 3.20)
project(Tensile VERSION 0.1.0 LANGUAGES CXX)

# True when Tensile is built on its own, false when a consumer pulls it in with add_subdirectory/FetchContent.
# (PROJECT_IS_TOP_LEVEL does the same but needs CMake 3.21.)
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(TENSILE_IS_TOP_LEVEL ON)
else ()
  set(TENSILE_IS_TOP_LEVEL OFF)
endif ()

# Tests, benchmarks and sanitizers are for developing Tensile itself, so they default to off for consumers.
option(TENSILE_BUILD_TESTS "Build the Tensile unit tests" ${TENSILE_IS_TOP_LEVEL})
option(TENSILE_BUILD_BENCHMARKS "Build the Tensile benchmarks" ${TENSILE_IS_TOP_LEVEL})

if (TENSILE_IS_TOP_LEVEL)
  set(TENSILE_DEFAULT_SANITIZERS "address,undefined")
else ()
  set(TENSILE_DEFAULT_SANITIZERS "")
endif ()
set(TENSILE_SANITIZERS "${TENSILE_DEFAULT_SANITIZERS}" CACHE STRING
    "Comma-separated -fsanitize list for Tensile's tests and benchmarks in Debug builds (empty disables)")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------------------------------------------------
# The library
# ---------------------------------------------------------------------------------------------------------------------

add_library(tensile INTERFACE)
add_library(tensile::tensile ALIAS tensile)

target_include_directories(tensile INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(tensile INTERFACE cxx_std_23)

# ---------------------------------------------------------------------------------------------------------------------
# Development targets (tests, benchmarks)
# ---------------------------------------------------------------------------------------------------------------------

# Applies the sanitizers to one target in Debug builds only, instead of changing global compiler flags.
# -fno-sanitize-recover makes UBSan abort on the first error so the failing test is reported as failed.
function(tensile_enable_sanitizers target)
  if (TENSILE_SANITIZERS)
    set(flags -fsanitize=${TENSILE_SANITIZERS} -fno-sanitize-recover=all -fno-omit-frame-pointer)
    target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:${flags}>)
    target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:-fsanitize=${TENSILE_SANITIZERS}>)
  endif ()
endfunction()

if (TENSILE_BUILD_TESTS OR TENSILE_BUILD_BENCHMARKS)
  include(FetchContent)
endif ()

if (TENSILE_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_subdirectory(tests)
endif ()

if (TENSILE_BUILD_BENCHMARKS)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.8.3
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googlebenchmark)

  add_subdirectory(benchmarks)
endif ()
