#ifndef SRC_SLAB_SLAB_OVERRIDES_HPP_
#define SRC_SLAB_SLAB_OVERRIDES_HPP_

/// @file slab_overrides.hpp
/// @brief Global operator new/delete overrides routed through the slab.
/// Include in exactly ONE translation unit.

#include "slab/slab_alloc.hpp"
#include <new>

void* operator new(std::size_t size) {
    void* p = slab::allocate(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size) {
    void* p = slab::allocate(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void*) noexcept {}  // NOLINT(readability/casting)
void operator delete[](void*) noexcept {}  // NOLINT(readability/casting)

void operator delete(void* ptr, std::size_t size) noexcept {
    slab::deallocate(ptr, size);  // GCOVR_EXCL_BR_LINE
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    slab::deallocate(ptr, size);  // GCOVR_EXCL_BR_LINE
}

#endif  // SRC_SLAB_SLAB_OVERRIDES_HPP_
