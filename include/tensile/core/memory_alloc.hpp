// This file will manage the contiguous block of memory utilizing the Rule of Five

#pragma once
#include <cassert>
#include <memory>
#include <new>
#include <utility>

// This class is meant to allocate the memory for m_size elements of type T in the heap with an alignment of 64 bytes
template <typename T, std::size_t Alignment = 64>
class MemStorage {
   private:

    // Pointer to the allocated storage
    T* m_data;
    // Size of contiguous memory
    std::size_t m_size;

    // Allocates raw memory with alignment of 64 bytes
    void alloc(size_t size) {
        m_data = static_cast<T*>(::operator new(size * sizeof(T), std::align_val_t{Alignment}));
    }

   public:
    // Default constructor
    MemStorage() noexcept : m_data{nullptr}, m_size{0} {}

    // Default constructor with provided size
    explicit MemStorage(const std::size_t size) : m_size{size} {
        assert(m_size >= 1);
        alloc(m_size);
    }

    // Destructor
    ~MemStorage() { ::operator delete(m_data, std::align_val_t{Alignment}); }

    // Copy constructor
    MemStorage(const MemStorage& other) : m_size(other.m_size) {
        alloc(m_size);
        std::uninitialized_copy(other.m_data, other.m_data + m_size, m_data);
    }

    // Copy assignment
    MemStorage& operator=(const MemStorage& other) {
        if (this == &other) {
            return *this;
        }

        MemStorage temp(other);
        std::swap(m_data, temp.m_data);
        std::swap(m_size, temp.m_size);

        return *this;
    }

    // Move constructor
    MemStorage(MemStorage&& other) noexcept : m_data{other.m_data}, m_size{other.m_size} {
        other.m_size = 0;
        other.m_data = nullptr;
    }

    // Move assignment
    MemStorage& operator=(MemStorage&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (m_data) {
            ::operator delete(m_data, std::align_val_t{Alignment});
        }

        m_size = other.m_size;
        m_data = other.m_data;
        other.m_size = 0;
        other.m_data = nullptr;

        return *this;
    }

    // Accessors
    [[nodiscard]] auto data() noexcept -> T* { return m_data; }
    [[nodiscard]] auto data() const noexcept -> const T* { return m_data; }
    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_size; }
};
