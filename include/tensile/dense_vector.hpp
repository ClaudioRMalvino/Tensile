// This file will describe the 1D vector class and implementation

#pragma once
#include <cassert>
#include <cmath>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <span>
#include <stdexcept>
#include <type_traits>

#include "core/memory_alloc.hpp"

// Namespace tensile starts here
namespace tensile {

/**
 * @brief Element types that `DenseVector` can store.
 *
 * The type must be trivially copyable and trivially default constructible, because `DenseVector` stores its elements
 * in raw memory and never runs constructors or destructors on them.
 *
 * @code
 * static_assert(tensile::TrivialVectorElement<double>);        // OK
 * static_assert(!tensile::TrivialVectorElement<std::string>);  // not trivially copyable
 * @endcode
 */
template <typename T>
concept TrivialVectorElement = std::is_trivially_copyable_v<T> && std::is_trivially_default_constructible_v<T>;

/**
 * @brief Dense, heap-allocated 1D vector.
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

    /**
     * @brief Floating-point type returned by `norm2()`.
     *
     * `E` itself when `E` is a floating-point type, otherwise `double`, so that integer vectors get an exact
     * (non-truncated) norm.
     */
    using RealType = std::conditional_t<std::is_floating_point_v<E>, E, double>;

    /**
     * @brief Constructs an empty vector with no allocated buffer.
     *
     * @code
     * tensile::DenseVector<double> v;   // v.size() == 0
     * @endcode
     */
    DenseVector() : detail::MemStorage<E, Alignment>()
    {}

    /**
     * @brief Allocates a vector of `size` elements without initializing them.
     *
     * @warning The elements hold indeterminate values. Write every element before reading it, or use
     * `DenseVector(size, value)`, `zeros()` or `ones()` instead.
     *
     * @code
     * tensile::DenseVector<double> v(3);   // v.size() == 3, contents uninitialized
     * for (std::size_t i{}; i < v.size(); ++i) { v[i] = static_cast<double>(i); }   // v = [0.0, 1.0, 2.0]
     * @endcode
     *
     * @param size Number of elements to allocate. Must be greater than zero.
     */
    explicit DenseVector(const size_t size) : detail::MemStorage<E, Alignment>(size)
    { assert(size > 0); }

    /**
     * @brief Allocates a vector of `size` elements and sets every element to `value`.
     *
     * @code
     * tensile::DenseVector<int> v(4, 7);   // v = [7, 7, 7, 7]
     * @endcode
     *
     * @param size Number of elements to allocate.
     * @param value Value assigned to every element.
     */
    explicit DenseVector(const size_t size, const E value) : detail::MemStorage<E, Alignment>(size)
    {
        for (size_t idx{}; idx < size; ++idx) { data()[idx] = value; }
    }

    /**
     * @brief Constructs a vector from a braced list of values.
     *
     * The size of the vector equals the number of values in the list.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};   // v.size() == 3, v = [1.0, 2.0, 3.0]
     * @endcode
     *
     * @param list Values copied into the vector in order.
     */
    DenseVector(std::initializer_list<E> list) : detail::MemStorage<E, Alignment>(list.size())
    { std::uninitialized_copy(list.begin(), list.end(), data()); }

    /**
     * @brief Creates a vector with every element set to zero.
     *
     * @code
     * auto v = tensile::DenseVector<double>::zeros(3);   // v = [0.0, 0.0, 0.0]
     * @endcode
     *
     * @param size The size of the vector.
     * @return DenseVector initialized with zeros.
     */
    static DenseVector zeros(const size_t size)
    {
        DenseVector result(size, E{});
        return result;
    }

    /**
     * @brief Creates a vector with every element set to one.
     *
     * @code
     * auto v = tensile::DenseVector<int>::ones(3);   // v = [1, 1, 1]
     * @endcode
     *
     * @param size The size of the vector.
     * @return DenseVector initialized with ones.
     */
    static DenseVector ones(const size_t size)
    {
        DenseVector result(size, E{1});
        return result;
    }

    /**
     * @brief Sets every element of the vector to zero in place.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * v.set_to_zeros();   // v = [0.0, 0.0, 0.0]
     * @endcode
     */
    void set_to_zeros()
    {
        for (size_t idx{0}; idx < size(); idx++) { data()[idx] = E{}; }
    }

    /**
     * @brief Returns a mutable reference to the element at `index`, without bounds checking.
     *
     * The index is checked with `assert` only. In builds that define `NDEBUG` an out-of-range index is undefined
     * behavior. Use `at()` for a checked access.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * v[1] = 5.0;   // v = [1.0, 5.0, 3.0]
     * @endcode
     *
     * @param index Zero-based position. Must be less than `size()`.
     * @return Reference to the element at `index`.
     */
    [[nodiscard]] E& operator[](size_t index)
    {
        assert(index < size());
        return data()[index];
    }

    /**
     * @brief Returns a read-only reference to the element at `index`, without bounds checking.
     *
     * The index is checked with `assert` only. In builds that define `NDEBUG` an out-of-range index is undefined
     * behavior. Use `at()` for a checked access.
     *
     * @code
     * const tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * double x = v[2];   // x == 3.0
     * @endcode
     *
     * @param index Zero-based position. Must be less than `size()`.
     * @return Const reference to the element at `index`.
     */
    [[nodiscard]] const E& operator[](const size_t index) const
    {
        assert(index < size());
        return data()[index];
    }

    /**
     * @brief Returns a mutable reference to an element, with bounds checking and negative indexing.
     *
     * A negative index counts from the end, so `-1` is the last element and `-size()` is the first.
     *
     * @code
     * tensile::DenseVector<int> v{10, 20, 30};
     * v.at(0)  = 1;    // v = [1, 20, 30]
     * v.at(-1) = 3;    // v = [1, 20, 3]
     * v.at(3);         // throws std::out_of_range
     * @endcode
     *
     * @param index Position in the range `[-size(), size())`.
     * @return Reference to the selected element.
     * @throws std::out_of_range If `index` is outside `[-size(), size())`.
     */
    [[nodiscard]] E& at(const int index)
    {
        if ((index < 0) && (index + static_cast<int>(size()) >= 0)) { return data()[size() + index]; }
        if ((index >= static_cast<int>(size())) || (index + static_cast<int>(size()) < 0)) {
            throw std::out_of_range(std::format("Index {}, is out of range for DenseVector of size {}", index, size()));
        }
        return data()[static_cast<size_t>(index)];
    }

    /**
     * @brief Returns a read-only reference to an element, with bounds checking and negative indexing.
     *
     * A negative index counts from the end, so `-1` is the last element and `-size()` is the first.
     *
     * @code
     * const tensile::DenseVector<int> v{10, 20, 30};
     * int first = v.at(0);    // 10
     * int last  = v.at(-1);   // 30
     * v.at(-4);               // throws std::out_of_range
     * @endcode
     *
     * @param index Position in the range `[-size(), size())`.
     * @return Const reference to the selected element.
     * @throws std::out_of_range If `index` is outside `[-size(), size())`.
     */
    [[nodiscard]] const E& at(const int index) const
    {
        if ((index < 0) && (index + (static_cast<int>(size())) >= 0)) { return data()[size() + index]; }
        if ((index >= static_cast<int>(size())) || (index + static_cast<int>(size()) < 0)) {
            throw std::out_of_range(std::format("Index {}, is out of range for DenseVector of size {}", index, size()));
        }
        return data()[static_cast<size_t>(index)];
    }

    /**
     * @brief Stream insertion operator for printing the vector.
     *
     * @code
     * tensile::DenseVector<int> v{1, 2, 3};
     * std::cout << v;   // prints: [1, 2, 3]
     * @endcode
     *
     * @param os The output stream.
     * @param vec The DenseVector to print.
     * @return A reference to the output stream for chaining.
     */
    friend std::ostream& operator<<(std::ostream& os, const DenseVector& vec)
    {
        os << "[";
        for (std::size_t idx{}; idx < vec.size(); ++idx) {
            os << vec[idx];
            if (idx < vec.size() - 1) { os << ", "; }
        }
        os << "]";
        return os;
    }

    /**
     * @brief In-place element by element exponentiation.
     *
     * Called on a non-const vector, this overload modifies the vector. An exponent of zero sets every element to one.
     *
     * @code
     * tensile::DenseVector<int> v{1, 2, 3};
     * v.pow(2);   // v = [1, 4, 9]
     * v.pow(0);   // v = [1, 1, 1]
     * @endcode
     *
     * @param exponent Integer power to raise elements by. Must be non-negative.
     * @return *this
     */
    DenseVector& pow(const int exponent) noexcept
    {
        assert(exponent >= 0);
        if (exponent == 0) {
            for (size_t idx{0}; idx < size(); ++idx) { data()[idx] = E{1}; }
            return *this;
        }
        if (exponent == 1) { return *this; }

        for (size_t idx{0}; idx < size(); ++idx) {
            E val{data()[idx]};
            for (int exp{0}; exp < exponent - 1; ++exp) { data()[idx] *= val; }
        }
        return *this;
    }

    /**
     * @brief Raises element by element to the power of an integer exponent, leaving the vector unchanged.
     *
     * This overload is chosen only when the vector is `const`. It returns the same values the in-place overload
     * would produce, including a vector of ones for an exponent of zero.
     *
     * @code
     * const tensile::DenseVector<int> v{1, 2, 3};
     * auto cubed = v.pow(3);   // cubed = [1, 8, 27], v is unchanged
     * auto ones  = v.pow(0);   // ones  = [1, 1, 1]
     * @endcode
     *
     * @param exponent Integer to raise the elements to. Must be non-negative.
     * @return A DenseVector raised to exponent.
     */
    [[nodiscard]] DenseVector pow(const int exponent) const
    {
        DenseVector result(*this);
        result.pow(exponent);
        return result;
    }

    //-----------------------------
    // Scalar operations
    // ----------------------------

    /**
     * @brief In-place scalar addition.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * v += 1.0;   // v = [2.0, 3.0, 4.0]
     * @endcode
     *
     * @param scalar Value to add to each element.
     * @return *this
     */
    DenseVector& operator+=(const E scalar) noexcept
    {
        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] += scalar; }
        return *this;
    }

    /**
     * @brief In-place scalar subtraction.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * v -= 1.0;   // v = [0.0, 1.0, 2.0]
     * @endcode
     *
     * @param scalar Value to subtract from each element.
     * @return *this
     */
    DenseVector& operator-=(const E scalar) noexcept
    {
        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] -= scalar; }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * v *= 2.0;   // v = [2.0, 4.0, 6.0]
     * @endcode
     *
     * @param scalar Value to multiply each element by.
     * @return *this
     */
    DenseVector& operator*=(const E scalar) noexcept
    {
        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] *= scalar; }
        return *this;
    }

    /**
     * @brief In-place scalar division.
     *
     * @code
     * tensile::DenseVector<double> v{2.0, 4.0, 6.0};
     * v /= 2.0;   // v = [1.0, 2.0, 3.0]
     * @endcode
     *
     * @param scalar Value to divide each element by. Must not be zero.
     * @return *this
     */
    DenseVector& operator/=(const E scalar) noexcept
    {
        assert(scalar != E{} && "Division of DenseVector by zero");
        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] /= scalar; }
        return *this;
    }

    /**
     * @brief Scalar addition operation.
     *
     * @code
     * const tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b = a + 1.0;   // b = [2.0, 3.0, 4.0], a is unchanged
     * @endcode
     *
     * @param scalar Value to add to each element.
     * @return DenseVector with each element added by scalar.
     */
    [[nodiscard]] DenseVector operator+(const E scalar) const
    {
        DenseVector result(*this);
        result += scalar;
        return result;
    }

    /**
     * @brief Scalar subtraction operation.
     *
     * @code
     * const tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b = a - 1.0;   // b = [0.0, 1.0, 2.0], a is unchanged
     * @endcode
     *
     * @param scalar Value to subtract from each element.
     * @return DenseVector with each element subtracted by scalar.
     */
    [[nodiscard]] DenseVector operator-(const E scalar) const
    {
        DenseVector result(*this);
        result -= scalar;
        return result;
    }

    /**
     * @brief Scalar multiplication operation.
     *
     * @code
     * const tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b = a * 2.0;   // b = [2.0, 4.0, 6.0], a is unchanged
     * @endcode
     *
     * @param scalar Value to multiply each element by.
     * @return DenseVector with each element multiplied by scalar.
     */
    [[nodiscard]] DenseVector operator*(const E scalar) const
    {
        DenseVector result(*this);
        result *= scalar;
        return result;
    }

    /**
     * @brief Scalar division operation.
     *
     * @code
     * const tensile::DenseVector<double> a{2.0, 4.0, 6.0};
     * tensile::DenseVector<double> b = a / 2.0;   // b = [1.0, 2.0, 3.0], a is unchanged
     * @endcode
     *
     * @param scalar Value to divide each element by. Must not be zero.
     * @return DenseVector with each element divided by scalar.
     */
    [[nodiscard]] DenseVector operator/(const E scalar) const
    {
        DenseVector result(*this);
        result /= scalar;
        return result;
    }

    //-------------------------------------------------
    // DenseVector - Container arithmetic operations
    //-------------------------------------------------

    /**
     * @brief In-place vector addition with any container of equal size.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * std::vector<double> w{10.0, 20.0, 30.0};
     * v += w;   // v = [11.0, 22.0, 33.0]
     * @endcode
     *
     * @param other Contiguous sequence to add into `this` (element by element). Must have the same size as `this`.
     * @return *this
     */
    DenseVector& operator+=(std::span<const E> other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector addition");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] += other[idx]; }

        return *this;
    }

    /**
     * @brief In-place vector subtraction with any container of equal size.
     *
     * @code
     * tensile::DenseVector<double> v{11.0, 22.0, 33.0};
     * std::vector<double> w{10.0, 20.0, 30.0};
     * v -= w;   // v = [1.0, 2.0, 3.0]
     * @endcode
     *
     * @param other Contiguous sequence to subtract from `this` (element by element). Must have the same size as
     * `this`.
     * @return *this
     */
    DenseVector& operator-=(std::span<const E> other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector subtraction");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] -= other[idx]; }

        return *this;
    }

    /**
     * @brief In-place vector multiplication with any container of equal size.
     *
     * @code
     * tensile::DenseVector<double> v{1.0, 2.0, 3.0};
     * std::vector<double> w{2.0, 3.0, 4.0};
     * v *= w;   // v = [2.0, 6.0, 12.0]
     * @endcode
     *
     * @param other Contiguous sequence to multiply with `this` (element by element). Must have the same size as
     * `this`.
     * @return *this
     */
    DenseVector& operator*=(std::span<const E> other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector multiplication");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] *= other[idx]; }

        return *this;
    }

    /**
     * @brief In-place vector division with any container of equal size.
     *
     * @code
     * tensile::DenseVector<double> v{2.0, 6.0, 12.0};
     * std::vector<double> w{2.0, 3.0, 4.0};
     * v /= w;   // v = [1.0, 2.0, 3.0]
     * @endcode
     *
     * @param other Contiguous sequence to divide `this` by (element by element). Must have the same size as `this`.
     * @return *this
     */
    DenseVector& operator/=(std::span<const E> other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector division");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] /= other[idx]; }
        return *this;
    }

    /**
     * @brief Adds a container of the same size element by element.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * std::vector<double> b{10.0, 20.0, 30.0};
     *
     * tensile::DenseVector<double> c = a + b; // c = {11.0, 22.0, 33.0}
     * @endcode
     *
     * @param other Contiguous sequence of the same size as `this`.
     * @return DenseVector resulting from the element by element summation of `this` and `other`.
     */
    [[nodiscard]] DenseVector operator+(std::span<const E> other) const
    {
        DenseVector result(*this);
        result += other;
        return result;
    }

    /**
     * @brief Subtracts a container of the same size element by element.
     *
     * @code
     * tensile::DenseVector<double> a{11.0, 22.0, 33.0};
     * std::vector<double> b{10.0, 20.0, 30.0};
     *
     * tensile::DenseVector<double> c = a - b; // c = {1.0, 2.0, 3.0}
     * @endcode
     *
     * @param other Contiguous sequence of the same size as `this`.
     * @return DenseVector resulting from the element by element subtraction of `other` from `this`.
     */
    [[nodiscard]] DenseVector operator-(std::span<const E> other) const
    {
        DenseVector result(*this);
        result -= other;
        return result;
    }

    /**
     * @brief Multiplies by a container of the same size element by element.
     *
     * This is not the inner product. Use `dot()` for that.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * std::vector<double> b{2.0, 3.0, 4.0};
     *
     * tensile::DenseVector<double> c = a * b; // c = {2.0, 6.0, 12.0}
     * @endcode
     *
     * @param other Contiguous sequence of the same size as `this`.
     * @return DenseVector resulting from the element by element product of `this` and `other`.
     */
    [[nodiscard]] DenseVector operator*(std::span<const E> other) const
    {
        DenseVector result(*this);
        result *= other;
        return result;
    }

    /**
     * @brief Divides by a container of the same size element by element.
     *
     * @code
     * tensile::DenseVector<double> a{2.0, 6.0, 12.0};
     * std::vector<double> b{2.0, 3.0, 4.0};
     *
     * tensile::DenseVector<double> c = a / b; // c = {1.0, 2.0, 3.0}
     * @endcode
     *
     * @param other Contiguous sequence of the same size as `this`.
     * @return DenseVector resulting from the element by element division of `this` by `other`.
     */
    [[nodiscard]] DenseVector operator/(std::span<const E> other) const
    {
        DenseVector result(*this);
        result /= other;
        return result;
    }

    /**
     * @brief Computes the inner (dot) product with a container of the same size.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * std::vector<double> b{4.0, 5.0, 6.0};
     *
     * double d = a.dot(b); // 1*4 + 2*5 + 3*6 = 32.0
     * @endcode
     *
     * @param other Contiguous sequence of the same size as `this`.
     * @return The sum of the element by element products of `this` and `other`.
     */
    [[nodiscard]] E dot(std::span<const E> other) const
    {
        assert(size() == other.size() && "Size mismatch for vector-vector dot product");

        E sum{};
        for (size_t idx{0}; idx < size(); ++idx) { sum += data()[idx] * other[idx]; }
        return sum;
    }

    //-------------------------------------------------
    // DenseVector - DenseVector arithmetic operations
    //-------------------------------------------------

    /**
     * @brief In-place vector addition with another DenseVector of equal size.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b{10.0, 20.0, 30.0};
     * a += b;   // a = [11.0, 22.0, 33.0]
     * @endcode
     *
     * @param other DenseVector to add into `this` (element by element). Must have the same size as `this`.
     * @return *this
     */
    DenseVector& operator+=(const DenseVector& other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector addition");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] += other[idx]; }

        return *this;
    }

    /**
     * @brief In-place vector subtraction with another DenseVector of equal size.
     *
     * @code
     * tensile::DenseVector<double> a{11.0, 22.0, 33.0};
     * tensile::DenseVector<double> b{10.0, 20.0, 30.0};
     * a -= b;   // a = [1.0, 2.0, 3.0]
     * @endcode
     *
     * @param other DenseVector to subtract from `this` (element by element). Must have the same size as `this`.
     * @return *this
     */
    DenseVector& operator-=(const DenseVector& other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector subtraction");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] -= other[idx]; }

        return *this;
    }

    /**
     * @brief In-place vector multiplication with another DenseVector of equal size.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b{2.0, 3.0, 4.0};
     * a *= b;   // a = [2.0, 6.0, 12.0]
     * @endcode
     *
     * @param other DenseVector to multiply with `this` (element by element). Must have the same size as `this`.
     * @return *this
     */
    DenseVector& operator*=(const DenseVector& other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector multiplication");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] *= other[idx]; }

        return *this;
    }

    /**
     * @brief In-place vector division with another DenseVector of equal size.
     *
     * @code
     * tensile::DenseVector<double> a{2.0, 6.0, 12.0};
     * tensile::DenseVector<double> b{2.0, 3.0, 4.0};
     * a /= b;   // a = [1.0, 2.0, 3.0]
     * @endcode
     *
     * @param other DenseVector to divide `this` by (element by element). Must have the same size as `this`.
     * @return *this
     */
    DenseVector& operator/=(const DenseVector& other)
    {
        assert(size() == other.size() && "Size mismatch for vector-vector division");

        for (size_t idx{0}; idx < size(); ++idx) { data()[idx] /= other[idx]; }
        return *this;
    }

    /**
     * @brief Adds two DenseVectors of the same size element by element.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0, 4.0};
     * tensile::DenseVector<double> b{1.0, 2.0, 3.0, 4.0};
     *
     * tensile::DenseVector<double> c = a + b; // c = {2.0, 4.0, 6.0, 8.0}
     * @endcode
     *
     * @param other DenseVector of the same size as `this`.
     * @return DenseVector resulting from the element by element summation of `this` and `other`.
     */
    [[nodiscard]] DenseVector operator+(const DenseVector& other) const
    {
        DenseVector result(*this);
        result += other;
        return result;
    }

    /**
     * @brief Subtracts two DenseVectors of the same size element by element.
     *
     * @code
     * tensile::DenseVector<double> a{5.0, 7.0, 9.0};
     * tensile::DenseVector<double> b{1.0, 2.0, 3.0};
     *
     * tensile::DenseVector<double> c = a - b; // c = {4.0, 5.0, 6.0}
     * @endcode
     *
     * @param other DenseVector of the same size as `this`.
     * @return DenseVector resulting from the element by element subtraction of `other` from `this`.
     */
    [[nodiscard]] DenseVector operator-(const DenseVector& other) const
    {
        DenseVector result(*this);
        result -= other;
        return result;
    }

    /**
     * @brief Multiplies two DenseVectors of the same size element by element.
     *
     * This is not the inner product. Use `dot()` for that.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b{4.0, 5.0, 6.0};
     *
     * tensile::DenseVector<double> c = a * b; // c = {4.0, 10.0, 18.0}
     * @endcode
     *
     * @param other DenseVector of the same size as `this`.
     * @return DenseVector resulting from the element by element product of `this` and `other`.
     */
    [[nodiscard]] DenseVector operator*(const DenseVector& other) const
    {
        DenseVector result(*this);
        result *= other;
        return result;
    }

    /**
     * @brief Divides two DenseVectors of the same size element by element.
     *
     * @code
     * tensile::DenseVector<double> a{4.0, 10.0, 18.0};
     * tensile::DenseVector<double> b{4.0, 5.0, 6.0};
     *
     * tensile::DenseVector<double> c = a / b; // c = {1.0, 2.0, 3.0}
     * @endcode
     *
     * @param other DenseVector of the same size as `this`.
     * @return DenseVector resulting from the element by element division of `this` by `other`.
     */
    [[nodiscard]] DenseVector operator/(const DenseVector& other) const
    {
        DenseVector result(*this);
        result /= other;
        return result;
    }

    /**
     * @brief Computes the inner (dot) product of two DenseVectors of the same size.
     *
     * @code
     * tensile::DenseVector<double> a{1.0, 2.0, 3.0};
     * tensile::DenseVector<double> b{4.0, 5.0, 6.0};
     *
     * double d = a.dot(b); // 1*4 + 2*5 + 3*6 = 32.0
     * @endcode
     *
     * @param other DenseVector of the same size as `this`.
     * @return The sum of the element by element products of `this` and `other`.
     */
    [[nodiscard]] E dot(const DenseVector& other) const
    {
        assert(size() == other.size() && "Size mismatch for vector-vector dot product");

        E sum{};
        for (size_t idx{0}; idx < size(); ++idx) { sum += data()[idx] * other[idx]; }
        return sum;
    }

    /**
     * @brief Computes the Euclidean (L2) norm of the vector.
     *
     * Returns the square root of the sum of squared elements. The sum is accumulated in `RealType`, so integer
     * vectors return a `double` and their norm is neither truncated nor prone to integer overflow.
     *
     * @code
     * tensile::DenseVector<double> v{3.0, 4.0};
     * double n = v.norm2(); // sqrt(3*3 + 4*4) = 5.0
     *
     * tensile::DenseVector<int> w{1, 1};
     * double m = w.norm2(); // sqrt(2) = 1.41421..., not 1
     * @endcode
     *
     * @return The Euclidean length of the vector.
     */
    [[nodiscard]] RealType norm2() const noexcept
    {
        RealType sqrSum{};
        for (size_t idx{0}; idx < size(); ++idx) {
            const auto val{static_cast<RealType>(data()[idx])};
            sqrSum += val * val;
        }
        return std::sqrt(sqrSum);
    }
};

// -------------------------------------------------
// DenseVector Free Functions
// -------------------------------------------------

/**
 * @brief Out-of-place exponentiation.
 *
 * @code
 * tensile::DenseVector<int> v{1, 2, 3};
 * auto squared = tensile::pow(v, 2); // squared = [1, 4, 9], v is unchanged
 * @endcode
 *
 * @param vec The vector to copy and exponentiate.
 * @param exponent Integer power to raise elements to. Must be non-negative.
 * @return A new DenseVector containing the exponentiated values.
 */
template <typename E, std::size_t Alignment>
[[nodiscard]] DenseVector<E, Alignment> pow(const DenseVector<E, Alignment>& vec, const int exponent)
{
    return vec.pow(exponent);
}
}  // namespace tensile
