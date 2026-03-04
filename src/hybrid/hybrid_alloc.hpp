#ifndef SRC_HYBRID_HYBRID_ALLOC_HPP_
#define SRC_HYBRID_HYBRID_ALLOC_HPP_

/// @file hybrid_alloc.hpp
/// @brief Hybrid allocator — shared slab + multiple per-program arenas.
/// Small allocs (<= HYBRID_THRESHOLD) go to the shared slab.
/// Large allocs go to the arena identified by index
/// TODO: figure out how to actually do this indexing...
///
/// Macros (define before including):
///   HYBRID_THRESHOLD — max bytes routed to slab (default: 4096)
///   NUM_ARENAS       — number of arena instances  (default: 1)
///   ARENA_CAPACITY   — capacity of each arena     (default: 64 KiB)

#include "slab/slab_alloc.hpp"
#include "arena/arena_alloc.hpp"

#ifndef HYBRID_THRESHOLD
#define HYBRID_THRESHOLD MAX_SIZE
#endif

#ifndef NUM_ARENAS
#define NUM_ARENAS 1
#endif

namespace hybrid {

/// Per-program arena instances.
static arena::Arena arenas[NUM_ARENAS];

/// @brief Allocate `size` bytes.
/// Small requests (<= HYBRID_THRESHOLD) try slab first; on failure fall
/// back to the specified arena.  Large requests go straight to arena.
/// @param size  Number of bytes to allocate.
/// @param id    Arena index (0-based, must be < NUM_ARENAS).
static void* allocate(std::size_t size, std::size_t id = 0) {
    if (size == 0) size = 1;
    // Default to arena if slab couldn't work.
    // We will want to always pass in id, since
    // it could default to arena. And if we run
    // in parallel it would be bad.
    if (size <= HYBRID_THRESHOLD) {
        void* p = slab::allocate(size);
        if (p) return p;
    }
    if (id >= NUM_ARENAS) return nullptr;  // GCOVR_EXCL_BR_LINE
    return arenas[id].allocate(size);
}

/// @brief Check whether a pointer lives inside the slab storage.
static bool in_slab(const void* ptr) {
    const char* p = static_cast<const char*>(ptr);
    return p >= slab::storage && p < slab::storage + slab::TOTAL_BYTES;  // GCOVR_EXCL_BR_LINE
}

/// @brief Deallocate a block previously obtained from allocate().
/// Uses pointer range check to route to the correct back-end.
static void deallocate(void* ptr, std::size_t size) {
    if (!ptr) return;
    if (size == 0) size = 1;
    // This is just to check in case a slab allocate went
    // to the arena instead.
    if (in_slab(ptr)) slab::deallocate(ptr, size);
    // arena deallocations are no-ops
}

/// @brief Get a reference to arena `id`.
static arena::Arena& get_arena(std::size_t id) {
    return arenas[id];
}

/// @brief Reset all arenas.
static void reset_arenas() {
    for (std::size_t i = 0; i < NUM_ARENAS; ++i) arenas[i].reset();
}

}  // namespace hybrid

#endif  // SRC_HYBRID_HYBRID_ALLOC_HPP_
