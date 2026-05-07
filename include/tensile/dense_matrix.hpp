// This file will describe the 2D matrix class and implementation

#pragma once
#include "core/layout_desc.hpp"
#include "core/memory_alloc.hpp"

template<typename T, typename Layout = RowMajor, size_t Alignment = 64>
class DenseMatrix : private MemStorage<T, Alignment> {
private:
    LayoutDesc<Layout> layout_;
public:
    DenseMatrix(size_t rows, size_t cols) : MemStorage<T, Alignment>(rows * cols), layout_{rows, cols} {
        assert(rows > 0 && cols > 0);
    }
    [[nodiscard]] auto operator()(size_t i, size_t j) noexcept -> T& { return this->data()[layout_(i, j)]; }
    [[nodiscard]] auto operator()(size_t i, size_t j) const noexcept -> const T& { return this->data()[layout_(i, j)]; }

    [[nodiscard]] auto rows() const -> size_t { return layout_.rows(); }
    [[nodiscard]] auto cols() const -> size_t { return layout_.cols(); }
};
