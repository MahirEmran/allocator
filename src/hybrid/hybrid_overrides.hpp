#ifndef SRC_HYBRID_HYBRID_OVERRIDES_HPP_
#define SRC_HYBRID_HYBRID_OVERRIDES_HPP_

/// @file hybrid_overrides.hpp
/// @brief Global operator new/delete overrides through hybrid.

#include "hybrid/hybrid_alloc.hpp"

void* operator new(std::size_t size) {
    /// TODO: figure out how to do arena ids here
    void* p = hybrid::allocate(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size) {
    /// TODO: figure out how to do ids here
    void* p = hybrid::allocate(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void*) noexcept {}  // NOLINT(readability/casting)
void operator delete[](void*) noexcept {}  // NOLINT(readability/casting)

void operator delete(void* ptr, std::size_t size) noexcept {
    hybrid::deallocate(ptr, size);  // GCOVR_EXCL_BR_LINE
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    hybrid::deallocate(ptr, size);  // GCOVR_EXCL_BR_LINE
}

#endif  // SRC_HYBRID_HYBRID_OVERRIDES_HPP_
