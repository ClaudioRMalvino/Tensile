// This file will describe the 1D vector class and implementation

#pragma once
#include "core/memory_alloc.hpp"

template<typename T, std::size_t Alignment = 64>
class DenseVector : private MemStorage<T, Alignment>
{

public:
    // Size allocation constructor
    explicit DenseVector(const size_t size): MemStorage<T, Alignment>(size) {}

    // List initialization constructor
    DenseVector(std::initializer_list<T> list): MemStorage<T, Alignment>(list.size()) {
        std::uninitialized_copy(list.begin(), list.end(), this->data());
    }

    using MemStorage<T,Alignment>::size;
    using MemStorage<T,Alignment>::data;

    // Element accessor/assigner
    [[nodiscard]] auto operator[](int index) -> T& {
        assert(index < this->size());
        if (index < 0) {
            return this->data()[this->size() + index];
        }
        return this->data()[index];
    }

    // Const element accessor/assigner
    [[nodiscard]] auto operator[](int index) const -> const T& {
        assert(index < this->size());
        if (index < 0) {
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

