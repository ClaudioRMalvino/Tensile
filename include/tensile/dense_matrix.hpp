// This file will describe the 2D matrix class and implementation

#pragma once
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <type_traits>

#include "core/layout_desc.hpp"
#include "core/memory_alloc.hpp"

// Namespace tensile starts here
namespace tensile {

/**
 * @brief Element types that `DenseMatrix` can store.
 *
 * The type must be trivially copyable and trivially default constructible, because `DenseMatrix` stores its elements
 * in raw memory and never runs constructors or destructors on them.
 *
 * @code
 * static_assert(tensile::TrivialMatrixElement<float>);         // OK
 * static_assert(!tensile::TrivialMatrixElement<std::string>);  // not trivially copyable
 * @endcode
 */
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

    using detail::LayoutDesc<Layout>::loc;

    /**
     * @brief Validates a nested initializer list and returns its column count.
     *
     * Called from the list constructor's member initializers, so the list is checked before anything reads
     * `list.begin()[0]` or allocates storage.
     *
     * @param list Rows passed to the list constructor.
     * @return Length of the first row.
     * @throws std::invalid_argument If `list` has no rows or its first row is empty.
     */
    static size_t checked_cols(std::initializer_list<std::initializer_list<E>> list)
    {
        if (list.size() == 0 || list.begin()->size() == 0) {
            throw std::invalid_argument("DenseMatrix list initialization requires at least one row and one column.");
        }
        return list.begin()->size();
    }

   public:
    using detail::MemStorage<E, Alignment>::size;
    using detail::MemStorage<E, Alignment>::data;
    using detail::LayoutDesc<Layout>::rows;
    using detail::LayoutDesc<Layout>::cols;

    /**
     * @brief Constructs an empty matrix with no allocated buffer.
     *
     * @code
     * tensile::DenseMatrix<double> m;   // m.rows() == 0, m.cols() == 0, m.size() == 0
     * @endcode
     */
    DenseMatrix() : detail::MemStorage<E, Alignment>(), detail::LayoutDesc<Layout>()
    {
    }

    /**
     * @brief Allocates a square matrix of `sqr_length * sqr_length` elements without initializing them.
     *
     * @warning The elements hold indeterminate values. Write every element before reading it, or use the value
     * constructor, `zeros()`, `ones()` or `identity()` instead.
     *
     * @code
     * tensile::DenseMatrix<double> m(3);   // 3 rows, 3 columns, contents uninitialized
     * m.set_to_zeros();                    // now every element is 0.0
     * @endcode
     *
     * @param sqr_length Number of rows and of columns. Must be greater than zero.
     */
    explicit DenseMatrix(const size_t sqr_length)
        : detail::MemStorage<E, Alignment>(sqr_length * sqr_length), detail::LayoutDesc<Layout>(sqr_length, sqr_length)
    { assert(sqr_length > 0); }

    /**
     * @brief Allocates a `rows` by `cols` matrix without initializing its elements.
     *
     * @warning The elements hold indeterminate values. Write every element before reading it, or use the value
     * constructor, `zeros()` or `ones()` instead.
     *
     * @code
     * tensile::DenseMatrix<double> m(2, 3);   // 2 rows, 3 columns, contents uninitialized
     * for (std::size_t i{}; i < m.rows(); ++i) {
     *     for (std::size_t j{}; j < m.cols(); ++j) { m[i, j] = 0.0; }
     * }
     * @endcode
     *
     * @param rows Number of rows. Must be greater than zero.
     * @param cols Number of columns. Must be greater than zero.
     */
    explicit DenseMatrix(const size_t rows, const size_t cols)
        : detail::MemStorage<E, Alignment>(rows * cols), detail::LayoutDesc<Layout>(rows, cols)
    { assert(rows > 0 && cols > 0); }

    /**
     * @brief Allocates a `row` by `col` matrix and sets every element to `value`.
     *
     * @code
     * tensile::DenseMatrix<int> m(2, 3, 7);   // [[7, 7, 7],
     *                                         //  [7, 7, 7]]
     * @endcode
     *
     * @param row Number of rows. Must be greater than zero.
     * @param col Number of columns. Must be greater than zero.
     * @param value Value assigned to every element.
     */
    explicit DenseMatrix(const size_t row, const size_t col, const E value)
        : detail::MemStorage<E, Alignment>(row * col), detail::LayoutDesc<Layout>(row, col)
    {
        assert(row > 0 && col > 0);
        for (size_t idx_i{}; idx_i < row; ++idx_i) {
            for (size_t idx_j{}; idx_j < col; ++idx_j) { data()[loc(idx_i, idx_j)] = value; }
        }
    }

    /**
     * @brief Constructs a matrix from a nested braced list, one inner list per row.
     *
     * The number of inner lists sets `rows()` and the length of the first inner list sets `cols()`. Every row must
     * have the same length. The values are written in row/column order regardless of `Layout`.
     *
     * @code
     * tensile::DenseMatrix<double> m{{1.0, 2.0, 3.0},
     *                                {4.0, 5.0, 6.0}};   // 2 rows, 3 columns
     *
     * tensile::DenseMatrix<double> bad{{1.0, 2.0},
     *                                  {3.0}};           // throws std::invalid_argument
     * @endcode
     *
     * @param list Rows of the matrix. Must contain at least one non-empty row.
     * @throws std::invalid_argument If `list` has no rows, the first row is empty, or the rows do not all have the
     * same length.
     */
    DenseMatrix(std::initializer_list<std::initializer_list<E>> list)
        : detail::MemStorage<E, Alignment>(list.size() * checked_cols(list)),
          detail::LayoutDesc<Layout>(list.size(), list.begin()->size())
    {
        for (size_t idx_i{}; idx_i < rows(); ++idx_i) {
            if (list.begin()[idx_i].size() != cols()) {
                throw std::invalid_argument("Ragged matrix form. Ensure that all rows are of the same length.");
            }

            for (size_t idx_j{}; idx_j < cols(); ++idx_j) {
                data()[loc(idx_i, idx_j)] = list.begin()[idx_i].begin()[idx_j];
            }
        }
    }

    /**
     * @brief Creates a `rows` by `cols` matrix with every element set to zero.
     *
     * @code
     * auto m = tensile::DenseMatrix<double>::zeros(2, 2);   // [[0.0, 0.0],
     *                                                        //  [0.0, 0.0]]
     * @endcode
     *
     * @param rows Number of rows. Must be greater than zero.
     * @param cols Number of columns. Must be greater than zero.
     * @return DenseMatrix initialized with zeros.
     */
    static DenseMatrix zeros(const size_t rows, const size_t cols)
    {
        DenseMatrix result(rows, cols, E{});
        return result;
    }

    /**
     * @brief Sets every element of the matrix to zero in place.
     *
     * @code
     * tensile::DenseMatrix<int> m{{1, 2},
     *                             {3, 4}};
     * m.set_to_zeros();   // [[0, 0],
     *                     //  [0, 0]]
     * @endcode
     */
    void set_to_zeros()
    {
        for (size_t idx_i{}; idx_i < rows(); ++idx_i) {
            for (size_t idx_j{}; idx_j < cols(); ++idx_j) { data()[loc(idx_i, idx_j)] = E{}; }
        }
    }

    /**
     * @brief Creates a `rows` by `cols` matrix with every element set to one.
     *
     * @code
     * auto m = tensile::DenseMatrix<int>::ones(2, 3);   // [[1, 1, 1],
     *                                                    //  [1, 1, 1]]
     * @endcode
     *
     * @param rows Number of rows. Must be greater than zero.
     * @param cols Number of columns. Must be greater than zero.
     * @return DenseMatrix initialized with ones.
     */
    static DenseMatrix ones(const size_t rows, const size_t cols)
    {
        DenseMatrix result(rows, cols, E{1});
        return result;
    }

    /**
     * @brief Creates a square identity matrix: ones on the main diagonal, zeros everywhere else.
     *
     * @code
     * auto m = tensile::DenseMatrix<double>::identity(3);   // [[1.0, 0.0, 0.0],
     *                                                        //  [0.0, 1.0, 0.0],
     *                                                        //  [0.0, 0.0, 1.0]]
     * @endcode
     *
     * @param sq_length Number of rows and of columns. Must be greater than zero.
     * @return Identity DenseMatrix of size `sq_length * sq_length`.
     */
    static DenseMatrix identity(const size_t sq_length)
    {
        DenseMatrix result(sq_length);

        for (size_t idx_i{}; idx_i < result.rows(); ++idx_i) {
            for (size_t idx_j{}; idx_j < result.cols(); ++idx_j) {
                if (idx_i == idx_j) {
                    result[idx_i, idx_j] = E{1};
                    continue;
                }
                result[idx_i, idx_j] = E{0};
            }
        }

        return result;
    }

    /**
     * @brief Returns a mutable reference to the element at row `idx_i`, column `idx_j`.
     *
     * Uses the C++23 multidimensional subscript, so call it as `m[i, j]`. The indices are checked with `assert`
     * only. In builds that define `NDEBUG` an out-of-range index is undefined behavior.
     *
     * @code
     * tensile::DenseMatrix<double> m{{1.0, 2.0},
     *                                {3.0, 4.0}};
     * m[0, 1] = 9.0;   // [[1.0, 9.0],
     *                  //  [3.0, 4.0]]
     * @endcode
     *
     * @param idx_i Row index. Must be less than `rows()`.
     * @param idx_j Column index. Must be less than `cols()`.
     * @return Reference to the element at `(idx_i, idx_j)`.
     */
    [[nodiscard]] E& operator[](const size_t idx_i, const size_t idx_j) noexcept
    { return data()[loc(idx_i, idx_j)]; }

    /**
     * @brief Returns a read-only reference to the element at row `idx_i`, column `idx_j`.
     *
     * Uses the C++23 multidimensional subscript, so call it as `m[i, j]`. The indices are checked with `assert`
     * only. In builds that define `NDEBUG` an out-of-range index is undefined behavior.
     *
     * @code
     * const tensile::DenseMatrix<double> m{{1.0, 2.0},
     *                                      {3.0, 4.0}};
     * double x = m[1, 0];   // x == 3.0
     * @endcode
     *
     * @param idx_i Row index. Must be less than `rows()`.
     * @param idx_j Column index. Must be less than `cols()`.
     * @return Const reference to the element at `(idx_i, idx_j)`.
     */
    [[nodiscard]] const E& operator[](const size_t idx_i, const size_t idx_j) const noexcept
    { return data()[loc(idx_i, idx_j)]; }

    /**
     * @brief Returns a tag object for the matrix's storage order.
     *
     * Useful for choosing an algorithm at compile time based on whether the buffer is row major or column major.
     *
     * @code
     * using Mat = tensile::DenseMatrix<double, tensile::ColMajor>;
     * static_assert(std::is_same_v<decltype(Mat::layout()), tensile::ColMajor>);
     * @endcode
     *
     * @return A default-constructed `Layout` (`RowMajor{}` or `ColMajor{}`).
     */
    [[nodiscard]]
    static constexpr Layout layout() noexcept
    { return {}; }

    /**
     * @brief Stream insertion operator for printing the DenseMatrix.
     *
     * Each row is printed on its own line, wrapped in brackets, with rows separated by commas. No newline follows
     * the closing bracket. An empty matrix prints as `[]`.
     *
     * @code
     * tensile::DenseMatrix<int> m{{1, 2},
     *                             {3, 4}};
     * std::cout << m;
     * // prints:
     * // [[1, 2],
     * //  [3, 4]]
     * @endcode
     *
     * @param os The output stream.
     * @param mat The DenseMatrix to print.
     * @return A reference to the output stream for chaining.
     */
    friend std::ostream& operator<<(std::ostream& os, const DenseMatrix& mat)
    {
        os << "[";
        for (std::size_t idx_i{}; idx_i < mat.rows(); ++idx_i) {
            if (idx_i > 0) { os << ",\n "; }
            os << "[";
            for (std::size_t idx_j{}; idx_j < mat.cols(); ++idx_j) {
                os << mat[idx_i, idx_j];
                if (idx_j < mat.cols() - 1) { os << ", "; }
            }
            os << "]";
        }
        os << "]";
        return os;
    }
};
}  // Namespace tensile
