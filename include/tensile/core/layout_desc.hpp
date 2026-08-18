// This file will handle the element access and converting from N-D to a flat 1D row or column major access
#pragma once
#include <cassert>
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
 * @tparam T
 */
template <Layout L>
class LayoutDesc {
   private:
    size_t rows_, cols_;

   public:
    // Default constructor
    LayoutDesc(size_t rows, size_t cols) : rows_(rows), cols_(cols) {}

    // Returns row size
    [[nodiscard]] size_t rows() const { return rows_; }
    // Returns column size
    [[nodiscard]] size_t cols() const { return cols_; }
    // Returns total size of the matrix
    [[nodiscard]] size_t size() const { return rows_ * cols_; }

    // operator() overload to provide the number of rows and columns which then is
    // arranged in either row or column major structure depending on the template argument
    [[nodiscard]] constexpr size_t operator()(size_t i, size_t j) const {
        assert(i < rows_ && j < cols_);
        if constexpr (std::is_same_v<L, RowMajor>) {
            return i * cols_ + j;
        } else {
            return j * rows_ + i;
        }
    }
};
}  // Namespace detail
}  // Namespace tensile