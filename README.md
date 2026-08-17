# Tensile

[![CI](https://github.com/ClaudioRMalvino/tensile/actions/workflows/workflow.yaml/badge.svg)](https://github.com/ClaudioRMalvino/tensile/actions/workflows/workflow.yaml)
[![Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B23)

> ⚠️ **Work in Progress:** Tensile is currently under active development. The API is subject to change, and core BLAS optimizations are in the process of being implemented and benchmarked.

**Tensile** is a high-performance, header-only linear algebra library written in modern C++23. It is designed to explore the intersection of clean C++ abstractions and hardware-level performance engineering.

The library focuses on dense matrices and vectors, featuring lazy evaluation via expression templates, cache-aware BLAS-equivalent operations, and zero-copy Python interoperability.

## Features (Current & Planned)

- **Header-Only Architecture:** Include `include/tensile.hpp` into your project and compile.
- **Modern C++23:** Leverages deducing `this` for clean CRTP expression templates and `std::mdspan` concepts for layout management.
- **Hardware Optimized:** Custom aligned memory allocators to guarantee SIMD-friendly cache line alignment (64-byte).
- **Expression Templates:** Complex arithmetic like $D = 2.0 * A + B - 0.5 * C$ is evaluated in a single pass without allocating temporary matrices.
- **High-Performance BLAS:** Progressively optimized Level 1, 2, and 3 operations featuring cache blocking, loop reordering, and register-level micro-kernels.
- **Decompositions:** LU (with partial pivoting), Cholesky, and QR factorizations.
- **Python Interop (Planned):** Native `pybind11` bindings providing zero-copy buffer sharing with NumPy.

##  Requirements

- **CMake** $\ge$ 3.20
- **C++23 Compliant Compiler** (GCC 13+, Clang 16+, or Apple Clang)
- Git (for fetching Google Test and Google Benchmark dependencies)

## Usage

Because Tensile is an `INTERFACE` library, integrating it into a CMake project is trivial:

```cmake
# Fetch or include the Tensile directory
add_subdirectory(tensile)

# Link your executable
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE tensile::tensile)