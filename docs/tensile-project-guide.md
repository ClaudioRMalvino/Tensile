# Project Guide: `tensile` — A Header-Only Linear Algebra Library in Modern C++

## What You Are Building

A header-only, templated linear algebra library for dense matrices and vectors in modern C++. The library will support lazy evaluation through expression templates, implement core BLAS-equivalent operations, and include matrix decompositions commonly used in scientific computing and quantitative finance. You will also provide Python bindings via pybind11 with zero-copy NumPy interoperability.

The name of the library is yours to choose. This document uses "tensile" as a placeholder.

---

## Why This Project Matters

Quantitative finance firms and scientific computing employers want developers who understand what happens between writing `C = A * B` and the hardware actually executing that multiply. This project forces you to confront every layer: how memory is laid out, how the CPU cache hierarchy affects performance, how floating-point arithmetic introduces error, and how modern C++ features can express all of this cleanly.

Building a library rather than an application also teaches API design. Every function signature, every template parameter, and every error message is a user-facing decision. Library authors think differently from application developers, and that mindset is highly valued.

---

## Prerequisites & Background Knowledge

Before you begin, you should be comfortable with the following. If any of these are weak spots, address them first — the recommended reading section at the end has targeted resources for each.

**C++ Fundamentals**
You need solid understanding of the Rule of Five (destructor, copy constructor, copy assignment, move constructor, move assignment), RAII, `const` correctness, and `noexcept` specifications. If you're unsure when to mark something `noexcept` or what happens when a move constructor throws, revise this before starting.

**Templates**
You should understand function templates, class templates, template specialisation, and template argument deduction. You don't need to be an expert in template metaprogramming yet — that's part of what this project teaches — but you should be able to write and use basic templated classes without difficulty.

**Linear Algebra**
You need working knowledge of matrix-vector multiplication, matrix-matrix multiplication, inner products, norms, and triangular systems. You should understand what LU, Cholesky, and QR factorisations are and when each is appropriate. If you've completed your MPhil coursework, this should be well in hand.

**Build Systems**
You should know enough CMake to define targets, set compiler flags, and link libraries. You don't need to be an expert — you'll learn the finer points as you go.

**Computer Architecture (Basics)**
Understanding of CPU caches (L1, L2, L3), cache lines, and why sequential memory access is faster than random access. A vague awareness is enough to start; Milestone 5 is where you'll develop deep intuition.

---

## Technology Choices

**Language standard:** C++20. Use C++17 as a fallback only if your compiler lacks a feature you need.

**Build system:** CMake. This is the industry standard for C++ projects. Define your library as an INTERFACE target since it's header-only.

**Testing:** Catch2 v3 or Google Test. Either is fine; pick one and stick with it. Pull it in via CMake's FetchContent so anyone can clone and build without manual dependency setup.

**Benchmarking:** Google Benchmark. Essential for Milestones 3–5 where performance measurement is the whole point.

**Python bindings:** pybind11. The standard choice for C++-to-Python binding in scientific computing.

**CI:** GitHub Actions. Run your tests across at least two compilers (GCC and Clang) with address sanitiser and undefined behaviour sanitiser enabled.

**Formatting and linting:** clang-format and clang-tidy. Pick a style and enforce it from day one.

---

## Repository Structure

Your repository should separate concerns cleanly. The library headers live in an `include/` directory with a subdirectory matching your library name. Tests, benchmarks, and Python bindings each get their own directory with their own CMakeLists. A `docs/` directory holds your design decisions document and benchmark results.

Within the library headers, organise by responsibility: core abstractions (storage, shape, type traits), dense container types (vector, matrix, views), expression template machinery, operations (BLAS levels 1, 2, 3), decompositions, and utilities (error handling, printing).

Think about this structure before writing any code. Sketch it on paper. The directory layout is your architecture diagram.

---

## Milestones

---

### Milestone 0 — Project Scaffolding

**Goal:** A fully functional build pipeline with CI, testing, and benchmarking infrastructure. No library code yet.

**What to do:**

Set up your Git repository, CMakeLists (top-level and per-subdirectory), clang-format config, clang-tidy config, and GitHub Actions workflow. Write a single trivial test that asserts `1 == 1` and a single trivial benchmark that measures nothing. Push, and make sure CI goes green on both GCC and Clang with sanitisers enabled.

**Why this comes first:**

Every subsequent milestone assumes you can compile, test, and benchmark with a single command. If your build system is broken or flaky, you'll waste enormous time debugging infrastructure instead of writing library code. Professional projects always invest in tooling before features.

**Hints and tips:**

- CMake's FetchContent module is the cleanest way to pull in Catch2 and Google Benchmark without requiring the user to install them manually. Read the CMake documentation for FetchContent carefully.
- For an INTERFACE library (header-only), you don't use `add_library(name STATIC ...)` or `add_library(name SHARED ...)`. Look up the INTERFACE keyword for `add_library` and understand what it means for include paths and compile features.
- Set your compiler warnings aggressively from the start. On GCC/Clang, think about `-Wall`, `-Wextra`, `-Wpedantic`, and `-Werror`. It's much easier to write clean code from the beginning than to fix a thousand warnings later.
- For your CI workflow, use a build matrix across compilers. Enable ASan and UBSan as separate test configurations.

**Definition of done:** Cloning the repo, running CMake configure, building, and running `ctest` all succeed. CI is green. A teammate could clone your repo and be running tests within five minutes.

---

### Milestone 1 — Core Abstractions

**Goal:** You can construct vectors and matrices, access their elements, and print them. No arithmetic.

**What to build:**

Four components: an aligned memory storage class, a shape/layout descriptor, a dense vector class, and a dense matrix class.

**Background — Memory Alignment:**

Modern CPUs load memory in cache-line-sized chunks (typically 64 bytes). If your data starts at an address that isn't a multiple of 64, a single vector operation might touch two cache lines instead of one, halving throughput. SIMD instructions (SSE, AVX) also have alignment requirements — misaligned loads are either slower or cause faults depending on the instruction. This is why `std::vector` isn't ideal for numerical work: it provides no alignment guarantees. Research aligned allocation in C++17 and platform-specific alternatives.

**Background — Memory Layout:**

A 2D matrix stored in contiguous memory can be arranged row-by-row (row-major, the C/C++ convention) or column-by-column (column-major, the Fortran/MATLAB/BLAS convention). The choice affects which operations are fast and which are slow, because iterating along the "wrong" dimension causes cache misses. Your library should support both layouts via a template parameter with tag types.

**Background — The Rule of Five:**

Your storage class owns raw heap memory. This means you must explicitly define or delete the destructor, copy constructor, copy assignment operator, move constructor, and move assignment operator. The copy-and-swap idiom is the standard technique for writing an exception-safe assignment operator — look it up and understand why it works.

**Hints and tips:**

- Resist the urge to use `std::vector` internally "for now" with plans to replace it later. You'll build habits and assumptions around its behaviour that will be painful to undo. Write the aligned storage class first.
- For element access, provide both a checked version (throws on out-of-bounds) and an unchecked version. Think about which one `operator()` should be and which one `operator[]` should be, and be prepared to justify your choice.
- Support `std::initializer_list` construction for both vectors and matrices. It makes testing dramatically more pleasant.
- Implement `operator<<` for ostream output early. You'll be printing matrices constantly during debugging and you want them formatted readably.
- Think carefully about what your matrix's `operator()` takes as arguments. Two `size_t` parameters for row and column is the standard convention in numerical libraries, as opposed to `operator[]` with a single index.
- Write tests for construction, element access, bounds checking, copy semantics, move semantics, and self-assignment. Self-assignment is the edge case that catches buggy Rule of Five implementations.

**Definition of done:** You can construct matrices and vectors of various sizes, access elements, print them, copy them, move them, and all tests pass under ASan and UBSan.

---

### Milestone 2 — Expression Templates

**Goal:** Arithmetic expressions like `D = 2.0 * A + B - 0.5 * C` compile and evaluate without creating temporary matrices for the intermediate results.

**Background — The Problem Expression Templates Solve:**

Without expression templates, `A + B` must return a fully materialised matrix. Then `(A + B) + C` creates *another* temporary. For a chain of operations, you allocate and fill multiple large temporaries that are immediately discarded. This is catastrophic for performance in numerical code.

Expression templates use the C++ type system to build a compile-time tree of the entire expression. No computation happens until you assign the result to a concrete matrix, at which point a single loop walks the tree and evaluates element-by-element into the destination. One allocation, one pass.

**Background — CRTP (Curiously Recurring Template Pattern):**

CRTP is the mechanism that makes expression templates work. A base class template is parameterised on its derived class, allowing it to call derived-class methods without virtual dispatch. Every expression type (concrete matrix, addition node, scalar multiplication node) inherits from a common CRTP base, giving you a uniform interface with zero runtime overhead. This is sometimes called "static polymorphism."

Read about CRTP independently before implementing. Understand how `static_cast<const Derived&>(*this)` works and why it's safe when the pattern is used correctly.

**What to build:**

A CRTP expression base that your Vector and Matrix classes inherit from. A binary expression node that captures two operands and an operation (addition, subtraction, element-wise multiplication) without evaluating. A scalar expression node for scalar-times-expression. Free-standing operator overloads that return expression nodes. And finally, constructors and assignment operators on Vector and Matrix that accept any expression and evaluate it.

**Hints and tips:**

- Start with just `operator+` between two matrices. Get that working end-to-end before adding other operations. The pattern is identical for subtraction and element-wise multiplication — once you have one, the others are mechanical.
- The expression nodes store *references* to their operands, not copies. This is essential for performance but introduces a well-known pitfall: if the operands go out of scope before the expression is evaluated, you get dangling references. Eigen has this same issue. Document it clearly and write a note in your design decisions about why you accepted this trade-off.
- Use `std::common_type_t` to handle mixed-precision expressions (e.g., `Matrix<float> + Matrix<double>` should produce `double`).
- To verify that no temporaries are created, instrument your storage class with a static allocation counter. After evaluating `D = A + B + C`, the counter should show exactly one allocation (for `D`).
- C++20 concepts are valuable here. Define a concept for "anything that behaves like a matrix expression" (has `rows()`, `cols()`, and `operator()(i, j)`) and constrain your templates with it. This dramatically improves error messages.

**Definition of done:** Compound expressions with mixed scalar and matrix operands evaluate correctly with a single allocation. Your instrumented test proves no temporaries are created.

---

### Milestone 3 — BLAS Level 1 (Vector-Vector Operations)

**Goal:** Implement the standard vector-vector operations (dot product, axpy, norms, scaling) with attention to both performance and numerical accuracy.

**Background — BLAS Levels:**

BLAS (Basic Linear Algebra Subprograms) is the standard interface for numerical linear algebra operations, organised into three levels. Level 1 covers vector-vector operations (O(n) work on O(n) data), Level 2 covers matrix-vector operations (O(n²) work on O(n²) data), and Level 3 covers matrix-matrix operations (O(n³) work on O(n²) data). The ratio of computation to memory access increases with each level, which has profound performance implications.

Level 1 operations are **memory-bandwidth-bound**: you can't do enough arithmetic per byte loaded to keep the CPU busy. The CPU spends most of its time waiting for data from memory. This means your optimisation target is throughput in bytes per second, not FLOPS.

**Background — Floating-Point Summation:**

Naive summation of many floating-point numbers accumulates rounding error proportional to the number of terms. For a dot product of two million-element vectors, this error can be significant. Kahan (compensated) summation is a well-known technique that tracks and corrects the accumulated rounding error, giving results accurate to nearly full machine precision at the cost of a few extra operations per element. Implement both naive and compensated variants and compare their accuracy on adversarial inputs.

**What to implement:**

The six standard BLAS Level 1 operations: inner product, axpy (y = alpha*x + y), Euclidean norm, absolute-value sum, scalar scaling, and index of maximum absolute value.

**Hints and tips:**

- Write benchmarks that report throughput in GiB/s, not just wall-clock time. Google Benchmark has a `SetBytesProcessed` method for exactly this. For dot product, you're reading two vectors, so the bytes processed is `2 * n * sizeof(T)`.
- On a modern machine, memory bandwidth is roughly 20–50 GiB/s. If your dot product is getting 5 GiB/s on large vectors, something is wrong (probably a missed optimisation or an unintended copy).
- Test edge cases: empty vectors, single-element vectors, vectors containing NaN or Inf, and very large vectors where rounding error in the naive implementation is detectable.
- Design your functions as free functions in a namespace, not as member functions. This follows the BLAS convention and is generally better API design for operations that take multiple arguments of the same type.
- Consider whether your BLAS1 functions should operate on raw pointers with a size, on your Vector type, or on any expression. Think about the trade-offs of each.

**Definition of done:** All six operations are correct, benchmarked, and documented. Kahan-compensated dot product demonstrably beats naive summation on a crafted adversarial input. Throughput on large vectors is within 80% of your machine's memory bandwidth.

---

### Milestone 4 — BLAS Level 2 (Matrix-Vector Operations)

**Goal:** Implement general matrix-vector multiply (`gemv`) and triangular matrix-vector multiply, and understand how memory layout affects performance.

**Background — Why Layout Matters Now:**

For Level 1 operations, memory layout is irrelevant — vectors are 1D. For matrix-vector multiplication, the layout of the matrix determines whether your inner loop accesses contiguous memory or strides across rows/columns. The performance difference can be 5–10× depending on matrix size and cache behaviour.

For a row-major matrix, computing `y = A * x` means each row of A is dot-producted with x. The row elements are contiguous in memory, so the inner loop has good spatial locality. For a column-major matrix, the natural approach is to accumulate scaled columns of A into y. Understanding both perspectives and implementing each efficiently is essential.

**What to implement:**

General matrix-vector multiply with the full BLAS signature (alpha, beta scaling). Triangular matrix-vector multiply for both upper and lower triangular matrices.

**Hints and tips:**

- Implement the naive version first for correctness, then optimise. The first optimisation is getting the loop ordering right for your memory layout. The second is cache blocking — processing the matrix in blocks that fit in L1 cache.
- Benchmark row-major vs column-major access patterns explicitly. Create the same matrix in both layouts, run gemv on both, and measure the throughput difference. Write up the results and explain them in terms of cache behaviour.
- For triangular multiply, think about how to indicate "upper" vs "lower" at the API level. Tag dispatch (empty struct types) and `if constexpr` are both clean approaches. Avoid using runtime booleans or enums if the choice is always known at compile time.
- Test against known results: identity matrix (should return the input vector unchanged), diagonal matrices, and small hand-computed cases.
- Your target for a 2048×2048 matrix should be within 50% of a reference BLAS implementation (OpenBLAS). If there's a gap, you should be able to explain where the performance is lost.

**Definition of done:** `gemv` works correctly for both layouts. Benchmarks show the performance difference between row-major and column-major access. Cache-blocked version shows measurable improvement over naive. You can articulate why the gap to OpenBLAS exists.

---

### Milestone 5 — BLAS Level 3 (Matrix-Matrix Multiplication)

**Goal:** Implement `gemm` and progressively optimise it from naive to high-performance. This is the most important milestone in the project.

**Background — Why gemm Is Special:**

Matrix-matrix multiplication is the only BLAS operation where the arithmetic intensity (FLOPS per byte loaded from memory) is high enough to be *compute-bound* rather than memory-bound. For an n×n multiply, you do O(n³) operations on O(n²) data. This means there's enough reuse potential that, with proper cache management, you can keep the CPU's floating-point units busy. The difference between a naive implementation and an optimised one is typically 10–30×.

**Background — The Cache Blocking Strategy:**

The key insight is that a small block of one matrix can be loaded into cache and reused many times while streaming through blocks of the other matrix. The goal is to decompose the multiplication into operations on tiles that fit in L1 or L2 cache. The GotoBLAS paper by Goto and van de Geijn (2008) is the foundational reference for how to structure these blocking levels. Read it before starting this milestone.

**Background — The Micro-Kernel:**

At the innermost level of the blocking hierarchy, you have a tiny fixed-size multiplication (e.g., 4×4 or 8×4 elements) that should be tuned to use all available registers. This "micro-kernel" is where the actual FLOPS happen. Everything else in the gemm is just data movement to feed it. Understanding this architecture is what separates people who have read about performance from people who have done it.

**Optimisation progression:**

Implement these stages one at a time, benchmarking after each. Do not move to the next stage until you understand the results of the current one.

1. **Naive implementation.** The textbook three-nested-loop version. Measure GFLOPS (computed as 2*M*N*K / time / 1e9). This will likely give you 1–3 GFLOPS.

2. **Loop reordering.** The order of the three loops (i, j, k) dramatically affects spatial locality. There are six permutations. For row-major matrices, one particular ordering is significantly better. Figure out which one and why.

3. **Cache tiling.** Decompose the multiplication into block operations sized to fit in L1 or L2 cache. Experiment with tile sizes. Your L1 is likely 32 KiB; think about how many doubles fit and what block dimensions that implies.

4. **Register blocking.** Within each tile operation, manually structure the innermost computation to operate on a small fixed-size panel that fits in CPU registers. This reduces loop overhead and enables instruction-level parallelism.

5. **(Stretch) SIMD intrinsics.** Replace the innermost arithmetic with AVX2 fused multiply-add instructions. This requires understanding how to load aligned data into SIMD registers and how to structure your micro-kernel to use them efficiently.

6. **(Stretch) Parallelism.** Add OpenMP parallelisation at the outermost blocking level. Measure scaling with core count.

**Hints and tips:**

- Measure in GFLOPS, not wall-clock time. This lets you compare against your CPU's theoretical peak (check your processor's spec sheet for its peak double-precision GFLOPS).
- Build a benchmark table: matrix size vs GFLOPS for each optimisation stage. Plot it. Include this in your documentation.
- The choice of tile size depends on your specific hardware's cache sizes. There is no single correct answer — experimentation is the point.
- For the register-blocking stage, think about what a 4×4 block of the output matrix needs: a 4×K panel of A and a K×4 panel of B. The 4×4 output block sits in registers and accumulates across K.
- Read the GotoBLAS paper and the BLIS project documentation before starting. These are not optional — they explain the blocking framework you're implementing.
- Reaching 30–40% of peak FLOPS without SIMD intrinsics is a good result. With intrinsics, 60%+ is achievable. Don't be discouraged if your first tiled implementation only gets 15% — that's normal, and the gap tells you where to focus.

**Definition of done:** You have a benchmark table showing GFLOPS across matrix sizes for each optimisation stage. Your best implementation achieves at least 30% of peak. You can walk through every optimisation stage, explain what it does, and point to the benchmark data showing its impact.

---

### Milestone 6 — Decompositions & Solvers

**Goal:** Implement LU, Cholesky, and QR factorisations using the operations you've already built.

**Background — Why These Three:**

LU with partial pivoting is the general-purpose workhorse for solving linear systems and computing determinants. Cholesky is the specialised variant for symmetric positive-definite matrices (covariance matrices in finance, Gram matrices in machine learning). QR via Householder reflections is the numerically stable foundation for least-squares problems. Together, these three cover the vast majority of dense linear algebra needs.

**What to implement:**

LU factorisation with partial pivoting, plus forward and back substitution to solve systems. Cholesky factorisation for SPD matrices. QR factorisation via Householder reflections, plus a least-squares solver built on top.

**Hints and tips:**

- Build your decompositions on top of your existing BLAS operations. LU should call your Level 2 and Level 3 operations internally, not re-implement raw loops. This validates your lower-level code and is how production LAPACK works.
- Partial pivoting in LU requires tracking row permutations. Think about how to represent the permutation and how to apply it efficiently.
- For Cholesky, decide what to do when the input is not positive definite. Detecting this mid-factorisation (a non-positive diagonal element appears) is the standard approach. Think about how to communicate this failure to the caller.
- For QR, use Householder reflections, not classical or modified Gram-Schmidt. Gram-Schmidt is numerically unstable for ill-conditioned matrices. Householder is backward stable. If you've studied numerical linear algebra, you'll know why — if not, read Trefethen & Bau's treatment.
- Test correctness by reconstruction: compute the factorisation, multiply the factors back together, and check that you recover the original matrix to near machine precision. Use the Frobenius norm of the residual divided by the norm of the original matrix as your error metric.
- Test with the Hilbert matrix at various sizes. It's famously ill-conditioned and will expose numerical issues. Document how your factorisations behave as the condition number grows.

**Definition of done:** All three factorisations pass reconstruction tests on well-conditioned matrices with relative residuals below 1e-12. Behaviour on ill-conditioned matrices is documented. The linear system solver gives accurate results verified against known solutions.

---

### Milestone 7 — Python Bindings

**Goal:** Expose your library to Python via pybind11 with zero-copy NumPy interoperability.

**Background — Why This Matters:**

Every quant desk you'll work on uses Python for prototyping, data analysis, and orchestration, with C++ underneath for performance-critical paths. The ability to build a high-performance C++ library and provide a clean Python interface is the exact workflow these firms use. pybind11 is the standard tool for this and is used by NumPy, SciPy, PyTorch, and most scientific Python packages.

**What to build:**

A pybind11 module that exposes your Vector, Matrix, and key operations (dot, gemv, gemm, factorisations) to Python. The critical feature is NumPy buffer protocol support, allowing zero-copy data sharing between your C++ types and NumPy arrays.

**Hints and tips:**

- pybind11's documentation is excellent. Read the "First steps" and "NumPy" sections end-to-end before writing anything.
- Python has no concept of templates, so you must explicitly instantiate the types you want to expose. Double precision is essential; single precision is a nice addition. Think about how to name the Python types.
- The buffer protocol is what makes this genuinely useful rather than a toy. If someone can create a Matrix in C++, pass it to Python as a NumPy array without copying, manipulate it, and pass it back — that's production-grade interop.
- Write your Python bindings in a separate directory with its own CMakeLists. The binding target will be a shared library (pybind11 modules always are). Keep the C++ library itself header-only and unchanged.
- Write Python tests using pytest. Test that round-tripping data between NumPy and your library preserves values exactly (bit-for-bit).
- Implement `__repr__`, `__len__`, `__getitem__`, and `__setitem__` to make your types feel Pythonic. Users shouldn't need to know they're calling into C++.
- Consider writing a small Python benchmark that compares your gemm against NumPy's (which calls into OpenBLAS or MKL). This is a great talking point.

**Definition of done:** You can `import tensile` in Python, create matrices, perform operations, and pass data to/from NumPy without copying. Python tests pass. A demo script shows a meaningful workflow.

---

### Milestone 8 — Hardening & Polish

**Goal:** Make the project genuinely portfolio-ready. This milestone is about quality, documentation, and presentation.

**What to do:**

**Compile-time safety.** Add concept constraints to every public template. A user who tries to instantiate your Matrix with a non-numeric type should get a clear, readable error — not pages of template instantiation failures.

**Exception safety.** Audit every public function. Determine whether it provides the no-throw guarantee, the strong guarantee, or the basic guarantee. Document this. HFT firms care about exception safety because throwing in a hot path is unacceptable.

**Test coverage.** Aim for 90%+ line coverage. Use gcov or lcov to measure. Identify untested paths and add tests. Your test suite should include exact-arithmetic checks on small matrices, known-answer comparisons against NumPy or MATLAB, reconstruction tests for decompositions, adversarial inputs for numerical stability, edge cases (empty, 1×1, non-square), and random stress tests.

**The design decisions document.** This is the most important document in your repository for employment purposes. Write a paragraph or two for each major design decision: why expression templates, why aligned storage, why CRTP over virtual dispatch, the row-major vs column-major trade-offs you observed, your cache blocking strategy and its measured impact, where your library is faster or slower than Eigen and why, and known limitations. Be honest about shortcomings — interviewers respect self-awareness far more than false confidence.

**README.** Include a one-paragraph summary, a quick-start example, a feature list, a benchmark plot, build instructions, and a link to the design decisions document. The README is the first thing a hiring manager sees.

**Definition of done:** CI is green on all compilers. clang-tidy runs clean. Test coverage is above 90%. The README and design decisions document are complete and polished. The repository is something you'd be proud to send in a job application.

---

## Stretch Goals

These are not necessary for a strong portfolio piece, but each adds significant depth if you have time.

**Compile-time fixed-size matrices.** Dimensions as template parameters, enabling stack allocation and full compile-time loop unrolling. Very Eigen-like.

**Eigenvalue solver.** The QR algorithm with shifts. Start with the symmetric case.

**Sparse matrix support.** Compressed sparse row format with sparse matrix-vector multiply. Relevant for PDE solvers.

**SIMD abstraction layer.** A small portable wrapper over SSE/AVX/AVX-512 that your micro-kernel uses. Teaches you about writing hardware-abstraction code.

**GPU offload.** Dispatch large gemm calls to cuBLAS via CUDA. Adds a GPU talking point to your CV.

---

## Recommended Reading

**Computer Architecture & Performance:**
- "What Every Programmer Should Know About Memory" by Ulrich Drepper. The definitive reference on cache behaviour, memory access patterns, and why your loop ordering matters. Read at least sections 2, 3, and 6 before Milestone 4.
- "Computer Architecture: A Quantitative Approach" by Hennessy & Patterson. The standard textbook. Chapters on memory hierarchy are directly relevant.

**Floating-Point Arithmetic:**
- "What Every Computer Scientist Should Know About Floating-Point Arithmetic" by David Goldberg (1991). Required reading before Milestone 3. Understand rounding modes, catastrophic cancellation, and why naive summation loses precision.

**High-Performance Linear Algebra:**
- "Anatomy of High-Performance Matrix Multiplication" by Kazushige Goto and Robert van de Geijn (2008). Read this before Milestone 5. It explains the blocking framework that every high-performance BLAS implementation uses.
- The BLIS project wiki on GitHub (flame/blis). The clearest modern explanation of the five-loop gemm structure. Their diagrams of the blocking hierarchy are invaluable.
- "Introduction to High Performance Scientific Computing" by Victor Eijkhout (free PDF). Covers BLAS, cache optimisation, and parallel computing from a practical perspective.

**Numerical Linear Algebra:**
- "Numerical Linear Algebra" by Trefethen and Bau. The standard graduate textbook. Read the chapters on LU, Cholesky, QR, and stability before Milestone 6.

**Modern C++:**
- "Effective Modern C++" by Scott Meyers. Covers move semantics, smart pointers, and template type deduction — all directly relevant.
- "C++ Templates: The Complete Guide" by Vandevoorde, Josuttis, and Gregor. The authoritative reference on expression templates, CRTP, and template metaprogramming.
- cppreference.com for C++20 concepts syntax and standard library features.

**Expression Templates Specifically:**
- Search for "expression templates C++" in the context of Eigen's internal documentation and the Blaze library's design papers. Both explain the technique clearly with motivating examples.

**pybind11:**
- The official pybind11 documentation at pybind11.readthedocs.io. Read "First steps," "Object-oriented code," and "NumPy" sections before Milestone 7.

**Eigen Source Code (for reference, not copying):**
- Study `Eigen/src/Core/products/` to see how a mature library structures its gemm. Don't replicate it — understand the ideas and implement your own version.

---

## Interview Talking Points This Project Gives You

When complete, you'll be able to speak confidently about template metaprogramming (expression templates, CRTP, concepts, `if constexpr`), memory management (Rule of Five, custom allocators, alignment, copy-and-swap), performance engineering (cache hierarchy, spatial and temporal locality, blocking, micro-kernels, bandwidth-bound vs compute-bound analysis), numerical computing (floating-point error, compensated summation, condition numbers, backward stability), C++ and Python interoperability (pybind11, buffer protocol, zero-copy), and software engineering (CMake, CI, sanitisers, testing strategies, API design).

Each of these maps directly to questions asked at quantitative finance firms, HFT shops, and scientific computing positions.

---

## General Advice

Build incrementally. Resist the urge to design the entire library up front — you'll make better decisions after you've written the earlier milestones and learned from them.

Benchmark obsessively. Every claim about performance should have a number attached. "I think this is faster" is worthless; "this is 3.2× faster on 1024×1024 doubles, here's the plot" is compelling.

Document as you go. Write your design decisions document alongside the code, not after. You'll forget your reasoning if you wait.

Commit frequently with descriptive messages. Your commit history tells a story. A reviewer should be able to read your commits and understand the progression of the project.

Don't chase Eigen's performance. Eigen has had twenty years of development. Getting within 50% of Eigen on gemm is an excellent result for a one-person project. What matters is that you understand *why* the gap exists and can articulate it.

The process of building this — not just the result — is what sets you apart.
