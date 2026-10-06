// This file will handle the element access and converting from N-D to a flat 1D row or column major access
#pragma once
#include<cassert>
#include <concepts>
#include <cstddef>
#include <type_traits>

// Namespace tensile::detail starts here
namespace tensile {

struct RowMajor {};
struct ColMajor {};

namespace detail {

// Only allows for these two template types so that DenseMatrix<double, Banana>
//  is not allowed to silently define the layout as ColMajor
template <typename T>
concept Layout = std::same_as<RowMajor, T> || std::same_as<ColMajor, T>;

/**
 * This template class defines the layout structure for DenseMatrix such as row size,
 * column size, and if the matrix will be row or column major.
 * @tparam L
 */
template <Layout L>
class LayoutDesc {
private:
    size_t rows_{}, cols_{};

public:
    // Default constructor
    LayoutDesc() = default;

    // Size allocated constructor
    LayoutDesc(size_t rows, size_t cols) : rows_(rows), cols_(cols) {}

    // Returns row size
    [[nodiscard]] size_t rows() const { return rows_; }
    // Returns column size
    [[nodiscard]] size_t cols() const { return cols_; }

    /**
     * Provides correct indexing for locating an element within the flattened 2D array
     * depending on if the instance is Row Major or Col Major.
     * @param idx_i: the row index
     * @param idx_j : the column index
     * @return
     */
    [[nodiscard]] constexpr size_t loc(size_t idx_i, size_t idx_j) const {
        assert(idx_i < rows_ && idx_j < cols_);
        if constexpr (std::is_same_v<L, RowMajor>) {
            return idx_i * cols_ + idx_j;
        } else {
            return idx_j * rows_ + idx_i;
        }
    }
};
}  // Namespace detail
}  // Namespace tensile
