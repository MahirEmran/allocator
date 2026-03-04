#ifndef SRC_ARENA_ARENA_ALLOC_HPP_
#define SRC_ARENA_ARENA_ALLOC_HPP_

/// @file arena_alloc.hpp
/// @brief Arena (bump-pointer) allocator with static storage.
/// Callers create their own Arena instances.
/// Override the buffer size with \#define ARENA_CAPACITY before including.

#include <cstddef>
#include <new>

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

/// @brief STL allocator wrapper that routes to a caller-supplied Arena.
/// TODO: I'm unsure of the implementation of this, I don't really like it much
/// I don't think it'd really work if we solely rely on arena
template <typename T>
struct ArenaSTL {
    typedef T value_type;
    Arena* arena_;  /// Pointer to the arena being used.

    /// @brief Construct with a reference to an Arena.
    explicit ArenaSTL(Arena& a) : arena_(&a) {}
    /// @brief Converting copy constructor for rebind.
    template <typename U> ArenaSTL(const ArenaSTL<U>& o)  // NOLINT(runtime/explicit)
        : arena_(o.arena_) {}

    T* allocate(std::size_t n) {
        void* p = arena_->allocate(n * sizeof(T));
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }
    void deallocate(T*, std::size_t) {}
};

template <typename T, typename U>
bool operator==(const ArenaSTL<T>&, const ArenaSTL<U>&) { return true; }
template <typename T, typename U>
bool operator!=(const ArenaSTL<T>&, const ArenaSTL<U>&) { return false; }

}  // namespace arena

#endif  // SRC_ARENA_ARENA_ALLOC_HPP_
