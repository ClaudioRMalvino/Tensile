// This file will describe the 2D matrix class and implementation

#pragma once
#include "core/layout_desc.hpp"
#include "core/memory_alloc.hpp"

// Namespace tensile starts here
namespace tensile {

template <typename T>
concept TrivialMatrixElement = std::is_trivially_copyable_v<T> && std::is_trivially_default_constructible_v<T>;

template <TrivialMatrixElement E, typename L = RowMajor, size_t Alignment = 64>
class DenseMatrix : private detail::MemStorage<E, Alignment> {
   private:
    detail::LayoutDesc<L> layout_;

   public:
    using detail::MemStorage<E, Alignment>::size;
    using detail::MemStorage<E, Alignment>::data;
    using layout_type = L;

    DenseMatrix(size_t rows, size_t cols) : detail::MemStorage<E, Alignment>(rows * cols), layout_{rows, cols} {
        assert(rows > 0 && cols > 0);
    }
    [[nodiscard]] E& operator()(size_t i, size_t j) noexcept { return this->data()[layout_(i, j)]; }
    [[nodiscard]] const E& operator()(size_t i, size_t j) const noexcept { return this->data()[layout_(i, j)]; }

    [[nodiscard]] size_t rows() const noexcept { return layout_.rows(); }
    [[nodiscard]] size_t cols() const noexcept { return layout_.cols(); }

    static constexpr layout_type layout() noexcept { return {}; }
};
}  // Namespace tensile
