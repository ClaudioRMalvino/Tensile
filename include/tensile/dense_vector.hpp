// This file will describe the 1D vector class and implementation

#pragma once
#include "core/memory_alloc.hpp"

template<typename T, std::size_t Alignment = 64>
class DenseVector : private MemStorage<T, Alignment>
{

public:

    // Default constructor
    explicit DenseVector(): MemStorage<T, Alignment>() {}

    // Size allocation constructor
    explicit DenseVector(const size_t size): MemStorage<T, Alignment>(size) {}

    // List initialization constructor
    DenseVector(std::initializer_list<T> list): MemStorage<T, Alignment>(list.size()) {
        std::uninitialized_copy(list.begin(), list.end(), this->data());
    }


    using MemStorage<T,Alignment>::size;
    using MemStorage<T,Alignment>::data;

    // Element accessor/assigner
    [[nodiscard]] auto operator[](size_t index) -> T& {
        return this->data()[index];
    }

    // Const element accessor/assigner
    [[nodiscard]] auto operator[](size_t index) const -> const T& {
        return this->data()[index];
    }

    // Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] auto at(const int index) const -> const T& {
        assert(index < static_cast<int>(this->size()));
        if (index < 0 && index + static_cast<int>(this->size()) >= 0) {
            return this->data()[this->size() + index];
        }
        return this->data()[index];
    }

    // Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] auto at(const int index) -> T& {
        assert(index < static_cast<int>(this->size()));
        if (index < 0 && index + static_cast<int>(this->size()) >= 0) {
            return this->data()[this->size() + index];
        }
        return this->data()[index];
    }

    // Self scalar additions
    void operator+=(const T scalar) {
        for (size_t i = 0; i < this->size(); ++i) {
            this->data()[i] += scalar;
        }
    }
    // Self scalar subtraction
    void operator-=(const T scalar) {
        for (size_t i = 0; i < this->size(); ++i) {
            this->data()[i] -= scalar;
        }
    }
    // Self scalar multiplication
    void operator*=(const T scalar) {
        for (size_t i = 0; i < this->size(); ++i) {
            this->data()[i] *= scalar;
        }
    }

    // Self scalar division
    void operator/=(const T scalar) {
        for (size_t i = 0; i < this->size(); ++i) {
            assert(this->data()[i] != 0);
            this->data()[i] /= scalar;
        }
    }
};

