#ifndef SRC_ARENA_ARENA_OVERRIDES_HPP_
#define SRC_ARENA_ARENA_OVERRIDES_HPP_

/// @file arena_overrides.hpp
/// @brief Global operator new/delete overrides using arena alloc.

#include "arena/arena_alloc.hpp"

/// Module-level arena used by the overridden operators.
static arena::Arena overrides_arena;

void* operator new(std::size_t size) {
    void* p = overrides_arena.allocate(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size) {
    void* p = overrides_arena.allocate(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void*) noexcept {}  // NOLINT(readability/casting)
void operator delete[](void*) noexcept {}  // NOLINT(readability/casting)
void operator delete(void*, std::size_t) noexcept {}
void operator delete[](void*, std::size_t) noexcept {}

#endif  // SRC_ARENA_ARENA_OVERRIDES_HPP_
