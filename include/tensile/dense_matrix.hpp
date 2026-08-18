// This file will describe the 2D matrix class and implementation

#pragma once
#include "core/layout_desc.hpp"
#include "core/memory_alloc.hpp"

// Namespace tensile starts here
namespace tensile {

    template <typename T>
        concept TrivialMatrixElement = requires(T)
    {
        std::is_trivially_copyable_v<T>;
        std::is_trivially_default_constructible_v<T>;
    };
    template<TrivialMatrixElement T, typename Layout = detail::RowMajor, size_t Alignment = 64>
    class DenseMatrix : private detail::MemStorage<T, Alignment> {
    private:
        detail::LayoutDesc<Layout> layout_;

    public:

        using value_type  = T;
        using layout_type = Layout;
        static constexpr size_t alignment = Alignment;

        DenseMatrix(size_t rows, size_t cols) : detail::MemStorage<T, Alignment>(rows * cols), layout_{rows, cols} {
            assert(rows > 0 && cols > 0);
        }
        [[nodiscard]] auto operator()(size_t i, size_t j) noexcept -> T& { return this->data()[layout_(i, j)]; }
        [[nodiscard]] auto operator()(size_t i, size_t j) const noexcept -> const T& { return this->data()[layout_(i, j)]; }

        [[nodiscard]] auto rows() const noexcept -> size_t { return layout_.rows(); }
        [[nodiscard]] auto cols() const noexcept -> size_t { return layout_.cols(); }

        static constexpr auto layout() noexcept -> layout_type { return {}; }
    };
} // Namespace tensile
