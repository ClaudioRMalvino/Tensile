cmake_mininum_required(VERSION 4.1)

project(tensile VERSION 0.1.0 LANGUAGES CXX PYTHON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -O3)

add_library(tensile INTERFACE)
target_include_directories(tensile INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(tensile INTERFACE cxx_std_23)








