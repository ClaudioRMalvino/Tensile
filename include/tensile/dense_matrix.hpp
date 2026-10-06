// This file will describe the 2D matrix class and implementation

#pragma once
#include <cassert>
#include <initializer_list>

#include <stdexcept>

#include "core/layout_desc.hpp"
#include "core/memory_alloc.hpp"

// Namespace tensile starts here
namespace tensile {

template <typename T>
concept TrivialMatrixElement = std::is_trivially_copyable_v<T> && std::is_trivially_default_constructible_v<T>;

/**
 * @brief Dense, heap-allocated 2D matrix that owns its elements.
 *
 * All `rows() * cols()` elements are stored in a single contiguous buffer whose address is aligned to `Alignment`
 * bytes. `Layout` decides the order of the elements inside that buffer; it does not change how the matrix is used.
 *
 * Indexing is `m[i, j]`, where `i` is the row and `j` is the column, for both layouts. Only the raw buffer returned
 * by `data()` exposes the storage order.
 *
 * @warning The size-only constructors allocate but do not initialize. Reading an element before writing to it is
 * undefined behavior. Use the value constructor, `zeros()` or `ones()` when initial values are needed.
 *
 * @warning Indices are checked with `assert` only. In builds that define `NDEBUG` an out-of-range index is
 * undefined behavior.
 *
 * Copying performs a deep copy of the buffer. Moving transfers the buffer and leaves the source empty.
 *
 * @tparam E Element type. Must be trivially copyable and trivially default constructible (e.g. `int`, `float`,
 * `double`), because the buffer is allocated as raw memory and elements are never individually constructed or
 * destroyed.
 * @tparam Layout Storage order of the buffer: `RowMajor` (default) stores each row contiguously, `ColMajor` stores
 * each column contiguously. Any other type is rejected at compile time.
 * @tparam Alignment Alignment of the buffer in bytes. Defaults to 64, the size of a typical cache line.
 *
 * @code
 * tensile::DenseMatrix<double> m{{1.0, 2.0, 3.0},
 *                                {4.0, 5.0, 6.0}};  // 2 rows, 3 columns
 * m[1, 0] = 7.0;                                    // row 1, column 0
 * double x = m[0, 2];                               // 3.0
 * @endcode
 */
template <TrivialMatrixElement E, typename Layout = RowMajor, size_t Alignment = 64>
class DenseMatrix : detail::MemStorage<E, Alignment>, detail::LayoutDesc<Layout> {
    static_assert(std::is_same_v<Layout, RowMajor> || std::is_same_v<Layout, ColMajor>,
                  "Layout must be RowMajor or ColMajor");

   private:
    using detail::LayoutDesc<Layout>::loc;

   public:
    using detail::MemStorage<E, Alignment>::size;
    using detail::MemStorage<E, Alignment>::data;
    using detail::LayoutDesc<Layout>::rows;
    using detail::LayoutDesc<Layout>::cols;

    // Default constructor
    explicit DenseMatrix() : detail::MemStorage<E, Alignment>(), detail::LayoutDesc<Layout>() {}

    /**
     * Size allocation constructor for a matrix of size = sqr_length * sqr_length.
     * @param sqr_length : const size_t desired length of row and column to construct a square matrix of size =
     * sqr_length * sq_length.
     */
    explicit DenseMatrix(const size_t sqr_length)
        : detail::MemStorage<E, Alignment>(sqr_length * sqr_length),
          detail::LayoutDesc<Layout>(sqr_length, sqr_length) {}

    /**
     * Size allocation constructor for rectangular shaped matrix.
     * @param rows : const size_t number of rows.
     * @param cols : const size_t number of columns.
     */
    explicit DenseMatrix(const size_t rows, const size_t cols)
        : detail::MemStorage<E, Alignment>(rows * cols), detail::LayoutDesc<Layout>(rows, cols) {}

    /**
     * Rectangular matrix constructor with elements initialized with `value`.
     * @param row : const size_t number of rows.
     * @param col : const size _t number of columns.
     * @param value : const E value with which to initialize all elements with.
     */
    explicit DenseMatrix(const size_t row, const size_t col, const E value)
        : detail::MemStorage<E, Alignment>(row * col), detail::LayoutDesc<Layout>(row, col)
    {
        for (size_t idx_i{}; idx_i < row; ++idx_i) {
            for (size_t idx_j{}; idx_j < col; ++idx_j) {
                data()[loc(idx_i, idx_j)] = value;
            }
        }
    }

    /**
     * List initialized constructor. Checks and throws an error for ragged matrices.
     * @param list : brace initialized list for constructing the DenseMatrix.
     */
    DenseMatrix(std::initializer_list<std::initializer_list<E>> list)
        : detail::MemStorage<E, Alignment>(list.size() * list.begin()[0].size()),
          detail::LayoutDesc<Layout>(list.size(), list.begin()[0].size()) {
        assert(list.size() > 0);

        for (size_t idx_i{}; idx_i < rows(); ++idx_i) {
            if (list.begin()[idx_i].size() != cols()) {
                throw std::invalid_argument("Ragged matrix form. Ensure that all columns are of the same size.");
            };

            for (size_t idx_j{}; idx_j < cols(); ++idx_j) {
                data()[loc(idx_i, idx_j)] = list.begin()[idx_i].begin()[idx_j];
            }
        }
    }

    /**
     * Static member which initializes a DenseMatrix of size = (rows * cols) with zero valued elements.
     * @param rows number of rows.
     * @param cols number of columns.
     * @return DenseMatrix initialized with zeros.
     */
    static DenseMatrix zeros(const size_t rows, const size_t cols) {
        DenseMatrix result(rows, cols, E{});
        return result;
    }

    /**
     * Sets all elements within the matrix equal to zero.
     */
    void set_to_zeros() {
        for (size_t idx_i{}; idx_i < rows(); ++idx_i)
        {
            for (size_t idx_j{}; idx_j < cols(); ++idx_j)
            {
                data()[loc(idx_i, idx_j)] = E{};
            }
        }
    }

    /**
     * Static member which initializes a DenseMatrix of size = (rows * cols) with all elements equal to unity.
     * @param rows number of rows.
     * @param cols number of columns.
     * @return DenseMatrix initialized with zeros.
     */
    static DenseMatrix ones(const size_t rows, const size_t cols) {
        DenseMatrix result(rows, cols, E{1});
        return result;
    }

    /**
     * Static memeber which initializes a square DenseMatrix with the main diagonal elements equal to unity.
     * @param sq_length the length of the rows and columns for the square matrix of size = sq_length * sq_length.
     * @return DenseMatrix with the main diagonal initialized with ones.
     */
    static DenseMatrix identity(const size_t sq_length) {

        DenseMatrix result(sq_length);

        for (size_t idx_i{}; idx_i < result.rows(); ++idx_i)
        {
            for (size_t idx_j{}; idx_j < result.cols(); ++idx_j)
            {
                if (idx_i == idx_j) {
                    result[idx_i, idx_j] = E{1};
                    continue;
                }
                result[idx_i, idx_j] = E{0};
            }
        }

        return result;
    }

    [[nodiscard]] E& operator[](const size_t idx_i, const size_t idx_j) noexcept { return data()[loc(idx_i, idx_j)]; }
    [[nodiscard]] const E& operator[](const size_t idx_i, const size_t idx_j) const noexcept {
        return data()[loc(idx_i, idx_j)];
    }
    [[nodiscard]]
    static constexpr Layout layout() noexcept {
        return {};
    }
};
}  // Namespace tensile
