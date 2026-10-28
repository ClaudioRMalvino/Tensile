#include <benchmark/benchmark.h>

#include <random>
#include <utility>

#include "tensile.hpp"

static void BM_DenseVectorArithmeticOps(benchmark::State& state)
{
    const auto size = static_cast<size_t>(state.range(0));
    tensile::DenseVector<double> dense_vector1(size, 1.0);
    tensile::DenseVector<double> dense_vector2(size, 2.0);
    tensile::DenseVector<double> dense_vector3(size, 3.0);
    // The input values must not be known to the compiler, or it could fold the arithmetic away.
    benchmark::DoNotOptimize(dense_vector1.data());
    benchmark::DoNotOptimize(dense_vector2.data());
    benchmark::DoNotOptimize(dense_vector3.data());

    for (auto _ : state) {
        tensile::DenseVector<double> dense_vector4 = dense_vector1 + dense_vector2 + dense_vector3;
        benchmark::DoNotOptimize(dense_vector4.data());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_DenseVectorArithmeticOps)->RangeMultiplier(10)->Range(1'000, 100'000'000);

static void BM_DenseVectorArithmeticOpsLoops(benchmark::State& state)
{
    const auto size = static_cast<size_t>(state.range(0));
    tensile::DenseVector<double> dense_vector1(size, 1.0);
    tensile::DenseVector<double> dense_vector2(size, 2.0);
    tensile::DenseVector<double> dense_vector3(size, 3.0);
    benchmark::DoNotOptimize(dense_vector1.data());
    benchmark::DoNotOptimize(dense_vector2.data());
    benchmark::DoNotOptimize(dense_vector3.data());

    for (auto _ : state) {
        tensile::DenseVector<double> dense_vector4(size);
        for (size_t idx_i{}; idx_i < size; ++idx_i) {
            dense_vector4[idx_i] = dense_vector1[idx_i] + dense_vector2[idx_i] + dense_vector3[idx_i];
        }
        benchmark::DoNotOptimize(dense_vector4.data());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_DenseVectorArithmeticOpsLoops)->RangeMultiplier(10)->Range(1'000, 100'000'000);

static void BM_DenseVectorArithmeticOpsLoopsPreallocated(benchmark::State& state)
{
    const auto size = static_cast<size_t>(state.range(0));
    tensile::DenseVector<double> dense_vector1(size, 1.0);
    tensile::DenseVector<double> dense_vector2(size, 2.0);
    tensile::DenseVector<double> dense_vector3(size, 3.0);
    tensile::DenseVector<double> dense_vector4(size, 0.0);
    benchmark::DoNotOptimize(dense_vector1.data());
    benchmark::DoNotOptimize(dense_vector2.data());
    benchmark::DoNotOptimize(dense_vector3.data());

    for (auto _ : state) {
        for (size_t idx_i{}; idx_i < size; ++idx_i) {
            dense_vector4[idx_i] = dense_vector1[idx_i] + dense_vector2[idx_i] + dense_vector3[idx_i];
        }
        benchmark::DoNotOptimize(dense_vector4.data());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_DenseVectorArithmeticOpsLoopsPreallocated)->RangeMultiplier(10)->Range(1'000, 100'000'000);

BENCHMARK_MAIN();
