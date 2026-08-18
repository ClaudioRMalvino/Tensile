// This file will handle the element access and converting from N-D to a flat 1D row or column major access
#pragma once
#include <cstddef>
#include <cassert>
#include <type_traits>

// Namespace tensile starts here
namespace tensile::detail {

    struct RowMajor {};
    struct ColMajor {};

    template <typename T>
    concept Layout = std::same_as<RowMajor, T> || std::same_as<ColMajor, T>;

    template <typename Layout>
    class LayoutDesc {
    private:
        size_t rows_, cols_;

    public:
        LayoutDesc(size_t rows, size_t cols) : rows_(rows), cols_(cols) {}
        [[nodiscard]] auto rows() const -> size_t { return rows_; }
        [[nodiscard]] auto cols() const -> size_t { return cols_; }
        [[nodiscard]] auto size() const -> size_t { return rows_ * cols_; }

        [[nodiscard]] constexpr auto operator()(size_t i, size_t j) const -> size_t {
            assert(i < rows_ && j < cols_);
            if constexpr (std::is_same_v<Layout, RowMajor>) {
                return i * cols_ + j;
            } else {
                return j * rows_ + i;
            }
        }
    };
} // Namespace tensile::detail