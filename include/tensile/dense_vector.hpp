// This file will describe the 1D vector class and implementation

#pragma once
#include <cmath>
#include <format>
#include <span>

#include "core/memory_alloc.hpp"

// Namespace tensile starts here
namespace tensile {

template <typename T>
concept TrivialVectorElement = std::is_trivially_copyable_v<T> && std::is_trivially_default_constructible_v<T>;

template <TrivialVectorElement E, std::size_t Alignment = 64>
class DenseVector : private detail::MemStorage<E, Alignment> {
   public:
    using detail::MemStorage<E, Alignment>::size;
    using detail::MemStorage<E, Alignment>::data;

    // Default constructor
    explicit DenseVector() : detail::MemStorage<E, Alignment>() {}

    // Size allocation constructor
    explicit DenseVector(const size_t size) : detail::MemStorage<E, Alignment>(size) {}

    // List initialization constructor
    DenseVector(std::initializer_list<E> list) : detail::MemStorage<E, Alignment>(list.size()) {
        std::uninitialized_copy(list.begin(), list.end(), this->data());
    }

    /**
     * Static member which initializes a DenseVector with zero elements of amount 'size'
     * @param size The size of the vector be instantiated and initialized with zeros
     * @return DenseVector initialized with zeros
     */
    static DenseVector zeros(const int size) noexcept {
        DenseVector result(size);
        for (int i{0}; i < size; ++i) {
            result[i] = 0;
        }
        return result;
    }

    /**
     * Resets all values within the vector to zeros
     * @return *this
     */
    DenseVector& setZeros() noexcept {
        for (size_t i{0}; i < this->size(); i++) {
            this->data()[i] = 0;
        }
        return *this;
    }

    /**
     * In-place element by element exponentiation
     * @param exponent Integer power to raise elements by
     * @return *this
     */
    DenseVector& pow(const int exponent) noexcept {
        assert(exponent >= 0);
        if (exponent == 0) {
            for (size_t i{0}; i < this->size(); ++i) {
                this->data()[i] = 1;
            }
            return *this;
        }
        if (exponent == 1) {
            return *this;
        }

        for (size_t i{0}; i < this->size(); ++i) {
            E val{this->data()[i]};
            for (int exp{0}; exp < exponent - 1; ++exp) {
                this->data()[i] *= val;
            }
        }
        return *this;
    }

    /**
     * Raises element by element to the power of an integer exponent
     * @param exponent Integer to raise the elements to
     * @return A DenseVector raised to exponent
     */
    DenseVector pow(const int exponent) const {
        assert(exponent >= 0);
        DenseVector result(*this);

        if (exponent == 1) {
            return result;
        }
        for (size_t i{0}; i < this->size(); ++i) {
            E val{result.data()[i]};
            for (int exp{0}; exp < exponent - 1; ++exp) {
                result.data()[i] *= val;
            }
        }
        return result;
    }

    // Element accessor/assigner without bound checking
    [[nodiscard]] E& operator[](size_t index) {
        assert(index < this->size());
        return this->data()[index];
    }

    // Const element accessor/assigner without bound checking
    [[nodiscard]] const E& operator[](const size_t index) const {
        assert(index < this->size());
        return this->data()[index];
    }

    // Non-Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] E& at(const int index) {
        assert(index < static_cast<int>(this->size()));
        if (index < 0 && index + static_cast<int>(this->size()) >= 0) {
            return this->data()[this->size() + index];
        }
        return this->data()[index];
    }

    // Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] const E& at(const int index) const {
        if (index < 0 && index + static_cast<int>(this->size()) >= 0) {
            return this->data()[this->size() + index];
        }
        if (index >= static_cast<int>(this->size()) || index + static_cast<int>(this->size()) < 0) {
            throw std::out_of_range(
                std::format("Index {}, is out of range for DenseVector of size {}", index, this->size()));
        }
        return this->data()[index];
    }

    /**
     * @brief Stream insertion operator for printing the vector
     * @param os The output stream
     * @param vec The DenseVector to print
     * @return A reference to the output stream for chaining
     */
    friend std::ostream& operator<<(std::ostream& os, const DenseVector& vec) {
        os << "[";
        for (std::size_t i{0}; i < vec.size(); ++i) {
            os << vec[i];
            if (i < vec.size() - 1) {
                os << ", ";
            }
        }
        os << "]";

        return os;
    }

    //-----------------------------
    // Scalar operations
    // ----------------------------

    /**
     * @brief In-place scalar addition
     * @param scalar value to add to each element
     * @return *this
     */
    DenseVector& operator+=(const E scalar) noexcept {
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
    DenseVector& operator-=(const E scalar) noexcept {
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
    DenseVector& operator*=(const E scalar) noexcept {
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
    DenseVector& operator/=(const E scalar) noexcept {
        for (size_t i{0}; i < this->size(); ++i) {
            assert(scalar != 0);
            this->data()[i] /= scalar;
        }
        return *this;
    }

    /**
     * @brief Scalar addition operation
     * @param scalar value to add to each element
     * @return DenseVector with each element added by scalar
     */
    [[nodiscard]] DenseVector operator+(const E scalar) {
        DenseVector result(*this);
        result += scalar;
        return result;
    }

    /**
     * @brief Scalar subtraction operation
     * @param scalar value to subtract from each element
     * @return DenseVector with each element subtracted by scalar
     */
    [[nodiscard]] DenseVector operator-(const E scalar) {
        DenseVector result(*this);
        result -= scalar;
        return result;
    }

    /**
     * @brief Scalar multiplication operation
     * @param scalar value to multiply each element by
     * @return DenseVector with each element multiplied by scalar
     */
    [[nodiscard]] DenseVector operator*(const E scalar) {
        DenseVector result(*this);
        result *= scalar;
        return result;
    }

    /**
     * @brief Scalar division operation
     * @param scalar value to divide each element by
     * @return DenseVector with each element divided by scalar
     */
    [[nodiscard]] DenseVector operator/(const E scalar) {
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
    DenseVector& operator+=(std::span<const E> other) {
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
    DenseVector& operator-=(std::span<const E> other) {
        assert(this->size() == other.size() && "Size mismatch for vector-vector subtraction");

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
    DenseVector& operator*=(std::span<const E> other) {
        assert(this->size() == other.size() && "Size mismatch for vector-vector multiplication");

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
    DenseVector& operator/=(std::span<const E> other) {
        assert(this->size() == other.size() && "Size mismatch for vector-vector division");

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] /= other[i];
        }
        return *this;
    }

    // DenseVector - container additions (element by element)
    [[nodiscard]] DenseVector operator+(std::span<const E> other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector addition");
        DenseVector result(*this);
        result += other;
        return result;
    }

    // DenseVector - container subtraction (element by element)
    [[nodiscard]] DenseVector operator-(std::span<const E> other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector subtraction");
        DenseVector result(*this);
        result -= other;
        return result;
    }

    // DenseVector - container multiplication (element by element)
    [[nodiscard]] DenseVector operator*(std::span<const E> other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector multiplication");
        DenseVector result(*this);
        result *= other;
        return result;
    }
    // DenseVector - container division (element by element)
    [[nodiscard]] DenseVector operator/(std::span<const E> other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector division");
        DenseVector result(*this);
        result /= other;
        return result;
    }

    [[nodiscard]] E dot(std::span<const E> other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector dot product");

        E sum{};
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
    DenseVector& operator+=(const DenseVector& other) {
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
    DenseVector& operator-=(const DenseVector& other) {
        assert(this->size() == other.size() && "Size mismatch for vector-vector subtraction");

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
    DenseVector& operator*=(const DenseVector& other) {
        assert(this->size() == other.size() && "Size mismatch for vector-vector multiplication");

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
    DenseVector& operator/=(const DenseVector& other) {
        assert(this->size() == other.size() && "Size mismatch for vector-vector division");

        for (size_t i{0}; i < this->size(); ++i) {
            this->data()[i] /= other[i];
        }
        return *this;
    }

    // DenseVector - container additions (element
    [[nodiscard]] DenseVector operator+(const DenseVector& other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector addition");
        DenseVector result(*this);
        result += other;
        return result;
    }

    // DenseVector - container subtraction
    [[nodiscard]] DenseVector operator-(const DenseVector& other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector subtraction");
        DenseVector result(*this);
        result -= other;
        return result;
    }

    // DenseVector - container multiplication
    [[nodiscard]] DenseVector operator*(const DenseVector& other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector multiplication");
        DenseVector result(*this);
        result *= other;
        return result;
    }

    [[nodiscard]] DenseVector operator/(const DenseVector& other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector division");
        DenseVector result(*this);
        result /= other;
        return result;
    }

    [[nodiscard]] E dot(const DenseVector& other) const {
        assert(this->size() == other.size() && "Size mismatch for vector-vector dot product");

        E sum{};
        for (size_t i{0}; i < this->size(); ++i) {
            sum += this->data()[i] * other[i];
        }
        return sum;
    }

    [[nodiscard]] E norm2() const noexcept {
        E sqrSum{};
        for (size_t i{0}; i < this->size(); ++i) {
            sqrSum += this->data()[i] * this->data()[i];
        }
        return std::sqrt(sqrSum);
    }
};

// -------------------------------------------------
// DenseVector Free Functions
// -------------------------------------------------

/**
 * @brief Out-of-place exponentiation
 * @param vec The vector to copy and exponentiate
 * @param exponent Integer power to raise elements to
 * @return A new DenseVector containing the exponentiated values
 */
template <typename E, std::size_t Alignment>
[[nodiscard]] DenseVector<E, Alignment> pow(const DenseVector<E, Alignment>& vec, const int exponent) {
    DenseVector<E, Alignment> result(vec);
    result.pow(exponent);
    return result;
}
}  // namespace tensile
