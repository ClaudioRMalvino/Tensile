// This file will describe the 1D vector class and implementation

#pragma once
#include <cmath>
#include <ostream>
#include "core/memory_alloc.hpp"

template<typename T, std::size_t Alignment = 64>
class DenseVector : private MemStorage<T, Alignment>
{

public:

    using MemStorage<T,Alignment>::size;
    using MemStorage<T,Alignment>::data;

    // Default constructor
    explicit DenseVector(): MemStorage<T, Alignment>() {}

    // Size allocation constructor
    explicit DenseVector(const size_t size): MemStorage<T, Alignment>(size) {}

    // List initialization constructor
    DenseVector(std::initializer_list<T> list): MemStorage<T, Alignment>(list.size()) {
        std::uninitialized_copy(list.begin(), list.end(), this->data());
    }

    // Element accessor/assigner
    [[nodiscard]] auto operator[](size_t index) -> T& {
        return this->data()[index];
    }

    // Const element accessor/assigner
    [[nodiscard]] auto operator[](const size_t index) const -> const T& {
        return this->data()[index];
    }

    // Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] auto at(const int index) const -> const T& {
        if (index < 0 && index + static_cast<int>(this->size()) >= 0) {
            return this->data()[this->size() + index];
        }
        if (index >= static_cast<int>(this->size()) || index + static_cast<int>(this->size()) < 0 )) {
            throw std::out_of_range(std::format(
                "Index {}, is out of range for DenseVector of size {}", index, this->size()
                ));
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

    //-----------------------------
    // Scalar operations
    // ----------------------------

    /**
     * @brief In-place scalar addition
     * @param scalar value to add to each element
     * @return *this
     */
    auto operator+=(const T scalar) noexcept -> DenseVector& {
        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] += scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar subtraction
     * @param scalar value to subtract from each element
     * @return *this
     */
    auto operator-=(const T scalar) noexcept -> DenseVector& {
        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] -= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication
     * @param scalar value to multiply each element by
     * @return *this
     */
    auto operator*=(const T scalar) noexcept -> DenseVector& {
        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar division
     * @param scalar value to divide each element by
     * @return *this
     */
    auto operator/=(const T scalar) noexcept -> DenseVector& {
        for (size_t i{0}; i < this->size(); ++i) {
            assert(this->data()[i] != 0);
            this->data()[i] /= scalar;
        }
        return *this;
    }

    /**
     * @brief Scalar addition operation
     * @param scalar value to add to each element
     * @return DenseVector with each element added by scalar
     */
    [[nodiscard]] auto operator+(const T scalar) -> DenseVector {
        DenseVector result(*this);
        result += scalar;
        return result;
    }

    /**
     * @brief Scalar subtraction operation
     * @param scalar value to subtract from each element
     * @return DenseVector with each element subtracted by scalar
     */
    [[nodiscard]] auto operator-(const T scalar) -> DenseVector {
        DenseVector result(*this);
        result -= scalar;
        return result;
    }

    /**
     * @brief Scalar multiplication operation
     * @param scalar value to multiply each element by
     * @return DenseVector with each element multiplied by scalar
     */
    [[nodiscard]] auto operator*(const T scalar) -> DenseVector {
        DenseVector result(*this);
        result *= scalar;
        return result;
    }

    /**
     * @brief Scalar division operation
     * @param scalar value to divide each element by
     * @return DenseVector with each element divided by scalar
     */
    [[nodiscard]] auto operator/(const T scalar) -> DenseVector {
        DenseVector result(*this);
        result /= scalar;
        return result;
    }

    //-------------------------------------------------
    // DenseVector - Container arithmetic operations
    //-------------------------------------------------

    /**
     * @brief In-place vector addition with any container of equal size
     * @param Vector to add into DenseVector (element by element)
     * @return *this
     */
     auto operator+=(std::span<const T> other) -> DenseVector& {
        assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t i{0}; i < this->size(); ++i) {
          this->data()[i] += other[i];
        }

        return *this;
    }

    /**
    * @brief In-place vector subtraction with any container of equal size
    * @param Vector to subtract from DenseVector (element by element)
    * @return *this
    */
    auto operator-=(std::span<const T> other) -> DenseVector& {
       assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] -= other[i];
        }

        return *this;
    }

    /**
    * @brief In-place vector multiplication with any container of equal size
    * @param Vector to multiply with DenseVector (element by element)
    * @return *this
    */
    auto operator*=(std::span<const T> other) -> DenseVector& {
        if (this->size() != other.size()) {
            throw std::runtime_error(std::format(
            "Size mismatch for vector-vector operation: lhs size = {}, rhs size = {}",
            this->size(),
            other.size()
            ));
        }

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] *= other[i];
        }

        return *this;
    }

    /**
    * @brief In-place vector division with any container of equal size
    * @param Vector to divide from DenseVector (element by element)
    * @return *this
    */
    auto operator/=(std::span<const T> other) -> DenseVector& {
       assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] /= other[i];
        }
        return *this;
    }

    // DenseVector - container additions (element
    [[nodiscard]] auto operator+(std::span<const T> other) const -> DenseVector {
        DenseVector result(*this);
        result += other;
        return result;
    }

    // DenseVector - container subtraction
    [[nodiscard]] auto operator-(std::span<const T> other) const -> DenseVector {
        DenseVector result(*this);
        result -= other;
        return result;
    }

    // DenseVector - container multiplication
    [[nodiscard]] auto operator*(std::span<const T> other) const -> DenseVector {
        DenseVector result(*this);
        result += other;
        return result;
    }

    [[nodiscard]] auto operator/(std::span<const T> other) const -> DenseVector {
        DenseVector result(*this);
        result += other;
        return result;
    }

    [[nodiscard]] auto dot(std::span<const T> other) const -> T {
        assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        T sum{};
        for (size_t i{0}; i < this->size(); ++i) {
            sum += this->data()[i] * other[i];
        }
        return sum;
    }

    //-------------------------------------------------
    // DenseVector - DenseVector arithmetic operations
    //-------------------------------------------------

    /**
     * @brief In-place vector addition with any container of equal size
     * @param Vector to add into DenseVector (element by element)
     * @return *this
     */
     auto operator+=(const DenseVector other) -> DenseVector& {
        assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t i{0}; i < this->size(); ++i) {
          this->data()[i] += other[i];
        }

        return *this;
    }

    /**
    * @brief In-place vector subtraction with any container of equal size
    * @param Vector to subtract from DenseVector (element by element)
    * @return *this
    */
    auto operator-=(const DenseVector other) -> DenseVector& {
       assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] -= other[i];
        }

        return *this;
    }

    /**
    * @brief In-place vector multiplication with any container of equal size
    * @param Vector to multiply with DenseVector (element by element)
    * @return *this
    */
    auto operator*=(const DenseVector other) -> DenseVector& {
        if (this->size() != other.size()) {
            throw std::runtime_error(std::format(
            "Size mismatch for vector-vector operation: lhs size = {}, rhs size = {}",
            this->size(),
            other.size()
            ));
        }

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] *= other[i];
        }

        return *this;
    }

    /**
    * @brief In-place vector division with any container of equal size
    * @param Vector to divide from DenseVector (element by element)
    * @return *this
    */
    auto operator/=(const DenseVector other) -> DenseVector& {
       assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] /= other[i];
        }
        return *this;
    }

    // DenseVector - container additions (element
    [[nodiscard]] auto operator+(const DenseVector other) const -> DenseVector {
        DenseVector result(*this);
        result += other;
        return result;
    }

    // DenseVector - container subtraction
    [[nodiscard]] auto operator-(const DenseVector other) const -> DenseVector {
        DenseVector result(*this);
        result -= other;
        return result;
    }

    // DenseVector - container multiplication
    [[nodiscard]] auto operator*(const DenseVector other) const -> DenseVector {
        DenseVector result(*this);
        result += other;
        return result;
    }

    [[nodiscard]] auto operator/(const DenseVector other) const -> DenseVector {
        DenseVector result(*this);
        result += other;
        return result;
    }

    [[nodiscard]] auto dot(const DenseVector other) const -> T {
        assert(this->size() == other.size() && "Size mismatch for vector-vector summation");

        T sum{};
        for (size_t i{0}; i < this->size(); ++i) {
            sum += this->data()[i] * other[i];
        }
        return sum;
    }

    [[nodiscard]] auto norm2() const noexcept -> T {
        T sqrSum{};
        for (int i{0}; i < this->size(); ++i) {
            sqrSum += this->data()[i] * this->data()[i];
        }
        return std::sqrt(sqrSum);
    }
};

