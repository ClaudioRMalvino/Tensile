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

/**
 * @brief Dense, heap-allocated 1D vector that owns its elements.
 *
 * All `size()` elements are stored in a single contiguous buffer whose address is aligned to `Alignment` bytes.
 *
 * Arithmetic operators act element by element: `a * b` multiplies matching elements and returns a vector of the
 * same size. Use `dot()` for the inner product. The right-hand side may be a scalar, another `DenseVector`, or any
 * contiguous sequence convertible to `std::span<const E>` (e.g. `std::vector<E>`). Both operands must have the same
 * size.
 *
 * @warning The size-only constructor allocates but does not initialize. Reading an element before writing to it is
 * undefined behavior. Use the value constructor, `zeros()` or `ones()` when initial values are needed.
 *
 * @warning `operator[]` and the size checks in the arithmetic operators use `assert` only. In builds that define
 * `NDEBUG` an out-of-range index or a size mismatch is undefined behavior.
 *
 * Copying performs a deep copy of the buffer. Moving transfers the buffer and leaves the source empty.
 *
 * @tparam E Element type. Must be trivially copyable and trivially default constructible (e.g. `int`, `float`,
 * `double`), because the buffer is allocated as raw memory and elements are never individually constructed or
 * destroyed.
 * @tparam Alignment Alignment of the buffer in bytes. Defaults to 64, the size of a typical cache line.
 *
 * @code
 * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
 * tensile::DenseVector<double> b{4.0, 5.0, 6.0};
 * a += b;                 // a is now [5.0, 7.0, 9.0]
 * double d = a.dot(b);    // 5*4 + 7*5 + 9*6 = 109.0
 * @endcode
 */
template <TrivialVectorElement E, std::size_t Alignment = 64>
class DenseVector : detail::MemStorage<E, Alignment> {
   public:
    using detail::MemStorage<E, Alignment>::size;
    using detail::MemStorage<E, Alignment>::data;

    // Default constructor
    explicit DenseVector() : detail::MemStorage<E, Alignment>() {}

    // Size allocation constructor
    explicit DenseVector(const size_t size) : detail::MemStorage<E, Alignment>(size) {}

    // Size allocation constructor with value initialization
    explicit DenseVector(const size_t size, const E value) : detail::MemStorage<E, Alignment>(size) {
        for (size_t idx{}; idx < size; ++idx) {
            this->data()[idx] = value;
        }
    }

    // List initialization constructor
    DenseVector(std::initializer_list<E> list) : detail::MemStorage<E, Alignment>(list.size()) {
        std::uninitialized_copy(list.begin(), list.end(), data());
    }

    /**
     * Static member which initializes a DenseVector with zero valued elements.
     *  @param size The size of the vector
     * @return DenseVector initialized with zeros
     */
    static DenseVector zeros(const size_t size) {
        DenseVector result(size, E{});
        return result;
    }

    /**
     * Static member which initializes a DenseVector with all elements valued to unity.
     * @param size The size of the vector
     * @return DenseVector initialized with ones
     */
    static DenseVector ones(const size_t size) {
        DenseVector result(size, E{1});
        return result;
    }

    /**
     * Sets all elements within the vector equal to zero for non-const DenseVectors.
     */
    void set_to_zeros() {
        for (size_t idx{0}; idx < size(); idx++) {
            data()[idx] = E{};
        }
    }

    // Element accessor/assigner without bound checking
    [[nodiscard]] E& operator[](size_t index) {
        assert(index < size());
        return data()[index];
    }

    // Const element accessor/assigner without bound checking
    [[nodiscard]] const E& operator[](const size_t index) const {
        assert(index < size());
        return data()[index];
    }

    // Non-Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] E& at(const int index) {

        if ((index < 0) && (index + static_cast<int>(size()) >= 0)) {
            return data()[size() + index];
        }
        if ((index >= static_cast<int>(size())) || (index + static_cast<int>(size()) < 0)) {
                    throw std::out_of_range(
                        std::format("Index {}, is out of range for DenseVector of size {}", index, size()));
                }
        return data()[static_cast<size_t>(index)];
    }

    // Const accesses to elements with bound checking and allows for negative indexing
    [[nodiscard]] const E& at(const int index) const {
        if ((index < 0) && (index + (static_cast<int>(size())) >= 0)) {
        return data()[size() + index];
        }
        if ((index >= static_cast<int>(size())) || (index + static_cast<int>(size()) < 0)) {
                   throw std::out_of_range(
                       std::format("Index {}, is out of range for DenseVector of size {}", index, size()));
        }
        return data()[index];
    }

    /**
     * @brief Stream insertion operator for printing the vector
     * @param os The output stream
     * @param vec The DenseVector to print
     * @return A reference to the output stream for chaining
     */
    friend std::ostream& operator<<(std::ostream& os, const DenseVector& vec) {
        os << "[";
        for (std::size_t idx{0}; idx < vec.size(); ++idx) {
            os << vec[idx];
            if (idx < vec.size() - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

    /**
     * In-place element by element exponentiation
     * @param exponent Integer power to raise elements by
     * @return *this
     */
    DenseVector& pow(const int exponent) noexcept {
        assert(exponent >= 0);
        if (exponent == 0) {
            for (size_t idx{0}; idx < size(); ++idx) {
                data()[idx] = 1;
            }
            return *this;
        }
        if (exponent == 1) {
            return *this;
        }

        for (size_t idx{0}; idx < size(); ++idx) {
            E val{data()[idx]};
            for (int exp{0}; exp < exponent - 1; ++exp) {
                data()[idx] *= val;
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
        for (size_t idx{0}; idx < size(); ++idx) {
            E val{result.data()[idx]};
            for (int exp{0}; exp < exponent - 1; ++exp) {
                result.data()[idx] *= val;
            }
        }
        return result;
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
        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] += scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar subtraction
     * @param scalar value to subtract from each element
     * @return *this
     */
    DenseVector& operator-=(const E scalar) noexcept {
        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] -= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication
     * @param scalar value to multiply each element by
     * @return *this
     */
    DenseVector& operator*=(const E scalar) noexcept {
        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar division
     * @param scalar value to divide each element by
     * @return *this
     */
    DenseVector& operator/=(const E scalar) noexcept {
        for (size_t idx{0}; idx < size(); ++idx) {
            assert(scalar != 0);
            data()[idx] /= scalar;
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
     * @param other to add into DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator+=(std::span<const E> other) {
        assert(size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] += other[idx];
        }

        return *this;
    }

    /**
     * @brief In-place vector subtraction with any container of equal size
     * @param other to subtract from DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator-=(std::span<const E> other) {
        assert(size() == other.size() && "Size mismatch for vector-vector subtraction");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] -= other[idx];
        }

        return *this;
    }

    /**
     * @brief In-place vector multiplication with any container of equal size
     * @param other to multiply with DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator*=(std::span<const E> other) {
        assert(size() == other.size() && "Size mismatch for vector-vector multiplication");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] *= other[idx];
        }

        return *this;
    }

    /**
     * @brief In-place vector division with any container of equal size
     * @param other to divide from DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator/=(std::span<const E> other) {
        assert(size() == other.size() && "Size mismatch for vector-vector division");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] /= other[idx];
        }
        return *this;
    }

    // DenseVector - container additions (element by element)
    [[nodiscard]] DenseVector operator+(std::span<const E> other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector addition");
        DenseVector result(*this);
        result += other;
        return result;
    }

    // DenseVector - container subtraction (element by element)
    [[nodiscard]] DenseVector operator-(std::span<const E> other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector subtraction");
        DenseVector result(*this);
        result -= other;
        return result;
    }

    // DenseVector - container multiplication (element by element)
    [[nodiscard]] DenseVector operator*(std::span<const E> other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector multiplication");
        DenseVector result(*this);
        result *= other;
        return result;
    }
    // DenseVector - container division (element by element)
    [[nodiscard]] DenseVector operator/(std::span<const E> other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector division");
        DenseVector result(*this);
        result /= other;
        return result;
    }

    [[nodiscard]] E dot(std::span<const E> other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector dot product");

        E sum{};
        for (size_t idx{0}; idx < size(); ++idx) {
            sum += data()[idx] * other[idx];
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
        assert(size() == other.size() && "Size mismatch for vector-vector summation");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] += other[idx];
        }

        return *this;
    }

    /**
     * @brief In-place vector subtraction with any container of equal size
     * @param Vector to subtract from DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator-=(const DenseVector& other) {
        assert(size() == other.size() && "Size mismatch for vector-vector subtraction");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] -= other[idx];
        }

        return *this;
    }

    /**
     * @brief In-place vector multiplication with any container of equal size
     * @param Vector to multiply with DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator*=(const DenseVector& other) {
        assert(size() == other.size() && "Size mismatch for vector-vector multiplication");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] *= other[idx];
        }

        return *this;
    }

    /**
     * @brief In-place vector division with any container of equal size
     * @param Vector to divide from DenseVector (element by element)
     * @return *this
     */
    DenseVector& operator/=(const DenseVector& other) {
        assert(size() == other.size() && "Size mismatch for vector-vector division");

        for (size_t idx{0}; idx < size(); ++idx) {
            data()[idx] /= other[idx];
        }
        return *this;
    }

    // DenseVector - container additions (element
    [[nodiscard]] DenseVector operator+(const DenseVector& other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector addition");
        DenseVector result(*this);
        result += other;
        return result;
    }

    // DenseVector - container subtraction
    [[nodiscard]] DenseVector operator-(const DenseVector& other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector subtraction");
        DenseVector result(*this);
        result -= other;
        return result;
    }

    // DenseVector - container multiplication
    [[nodiscard]] DenseVector operator*(const DenseVector& other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector multiplication");
        DenseVector result(*this);
        result *= other;
        return result;
    }

    [[nodiscard]] DenseVector operator/(const DenseVector& other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector division");
        DenseVector result(*this);
        result /= other;
        return result;
    }

    [[nodiscard]] E dot(const DenseVector& other) const {
        assert(size() == other.size() && "Size mismatch for vector-vector dot product");

        E sum{};
        for (size_t idx{0}; idx < size(); ++idx) {
            sum += data()[idx] * other[idx];
        }
        return sum;
    }

    [[nodiscard]] E norm2() const noexcept {
        E sqrSum{};
        for (size_t idx{0}; idx < size(); ++idx) {
            sqrSum += data()[idx] * data()[idx];
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
