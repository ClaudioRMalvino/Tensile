# Tensile

[![CI](https://github.com/ClaudioRMalvino/tensile/actions/workflows/workflow.yaml/badge.svg)](https://github.com/ClaudioRMalvino/tensile/actions/workflows/workflow.yaml)
[![Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B23)

> ⚠️ **Work in Progress:** Tensile is currently under active development. The API is subject to change, and core BLAS optimizations are in the process of being implemented and benchmarked.

**Tensile** is a high-performance, header-only linear algebra library written in modern C++23. It is designed to explore the intersection of clean C++ abstractions and hardware-level performance engineering.

The library focuses on dense matrices and vectors, featuring lazy evaluation via expression templates, cache-aware BLAS-equivalent operations, and zero-copy Python interoperability.

## Features (Current & Planned)

- **Header-Only Architecture:** Include `include/tensile.hpp` into your project and compile.
- **Modern C++23:** Uses C++20/23 concepts (rather than CRTP) to constrain element types, layouts and expression templates, and C++23 multidimensional subscripts (`m[i, j]`) for element access.
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
```

When Tensile is added as a subdirectory it only provides the headers: tests, benchmarks and their dependencies
(Google Test, Google Benchmark) are not downloaded or built. When building Tensile on its own they are enabled by
default and can be toggled:

| Option                     | Default (top-level / dependency) | Effect                                                       |
|----------------------------|----------------------------------|--------------------------------------------------------------|
| `TENSILE_BUILD_TESTS`      | `ON` / `OFF`                     | Build the unit tests                                         |
| `TENSILE_BUILD_BENCHMARKS` | `ON` / `OFF`                     | Build the benchmarks                                         |
| `TENSILE_SANITIZERS`       | `address,undefined` / empty      | Sanitizers for tests and benchmarks in Debug builds only     |

```sh
cmake -S . -B build-debug   -G Ninja -DCMAKE_BUILD_TYPE=Debug     # tests, with sanitizers
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release   # benchmarks
```

