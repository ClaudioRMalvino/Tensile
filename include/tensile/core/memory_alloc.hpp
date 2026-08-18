// This file will manage the contiguous block of memory utilizing the Rule of Five

#pragma once
#include <cassert>
#include <concepts>
#include <memory>
#include <new>
#include <utility>

// Namespace tensile::detail starts here
namespace tensile::detail {

template <typename T>
concept TrivialElement = std::is_trivially_copyable_v<T> && std::is_trivially_default_constructible_v<T>;

// This class is meant to allocate the memory for size_ elements of type T in the heap with an alignment of 64 bytes
template <TrivialElement E, std::size_t Alignment = 64>
class MemStorage {
   private:
    // Pointer to the allocated storage
    E* data_;
    // Size of contiguous memory
    std::size_t size_;

    // Allocates raw memory with alignment of 64 bytes
    // Matrices or uninitialized by default, reading into a newly constructed object without being
    // initialized will produce undefined behavior.
    void alloc(size_t size) { data_ = static_cast<E*>(::operator new(size * sizeof(E), std::align_val_t{Alignment})); }

   public:
    using value_type = E;
    static constexpr size_t alignment = Alignment;

    // Default constructor
    MemStorage() noexcept : data_{nullptr}, size_{0} {}

    // Default constructor with provided size
    explicit MemStorage(const std::size_t size) : size_{size} {
        assert(size_ > 0);
        alloc(size_);
    }

    // Destructor
    ~MemStorage() { ::operator delete(data_, std::align_val_t{Alignment}); }

    // Copy constructor
    MemStorage(const MemStorage& other) : size_(other.size_) {
        alloc(size_);
        std::uninitialized_copy(other.data_, other.data_ + size_, data_);
    }

    // Copy assignment
    MemStorage& operator=(const MemStorage& other) {
        if (this == &other) {
            return *this;
        }

        MemStorage temp(other);
        std::swap(data_, temp.data_);
        std::swap(size_, temp.size_);

        return *this;
    }

    // Move constructor
    MemStorage(MemStorage&& other) noexcept : data_{other.data_}, size_{other.size_} {
        other.size_ = 0;
        other.data_ = nullptr;
    }

    // Move assignment
    MemStorage& operator=(MemStorage&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (data_) {
            ::operator delete(data_, std::align_val_t{Alignment});
        }

        size_ = other.size_;
        data_ = other.data_;
        other.size_ = 0;
        other.data_ = nullptr;

        return *this;
    }

    // Accessors
    [[nodiscard]] E* data() noexcept { return data_; }
    [[nodiscard]] const E* data() const noexcept { return data_; }
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
};
}  // Namespace tensile::detail