#ifndef SRC_ARENA_ARENA_ALLOC_HPP_
#define SRC_ARENA_ARENA_ALLOC_HPP_

/// @file arena_alloc.hpp
/// @brief Arena (bump-pointer) allocator with static storage.
/// Callers create their own Arena instances.
/// Override the buffer size with \#define ARENA_CAPACITY before including.

#include <cstddef>

#ifndef ARENA_CAPACITY
#define ARENA_CAPACITY (64 * 1024)
#endif

namespace arena {

/// @brief Arena alloc, uses static ARENA_CAPACITY-byte buffer.
struct Arena {
    char buf[ARENA_CAPACITY];       /// Static backing storage
    std::size_t offset = 0;         /// Current write position

    /// @brief Allocate `size` bytes.
    /// @return Pointer to the region, or nullptr if out of space.
    void* allocate(std::size_t size) {
        if (offset + size > ARENA_CAPACITY) return nullptr;
        void* p = &buf[offset];
        offset += size;
        return p;
    }

    /// @brief Deallocate is a no-op; arena memory is freed only via reset().
    void deallocate(void*, std::size_t) {}

    /// @brief Reset the arena, reclaiming all memory.
    void reset() { offset = 0; }

    /// @brief Bytes currently in use.
    std::size_t used() const { return offset; }
    /// @brief Bytes still available.
    std::size_t remaining() const { return ARENA_CAPACITY - offset; }
    /// @brief Total capacity of this arena.
    static constexpr std::size_t capacity() { return ARENA_CAPACITY; }

    /// @brief Check whether `ptr` lies inside this arena's buffer.
    bool contains(const void* ptr) const {
        const char* p = static_cast<const char*>(ptr);
        return p >= buf && p < buf + ARENA_CAPACITY;  // GCOVR_EXCL_BR_LINE
    }
};

}  // namespace arena

#endif  // SRC_ARENA_ARENA_ALLOC_HPP_
