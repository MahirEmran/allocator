#ifndef SRC_SLAB_SLAB_ALLOC_HPP_
#define SRC_SLAB_SLAB_ALLOC_HPP_

/// @file slab_alloc.hpp
/// @brief Pool-based slab allocator with static storage.
/// Pre-partitions memory into fixed-size pools (free list for each size class)
/// All memory lives in a static char array.

#include <cstddef>

namespace slab {

/// Number of distinct size classes.
static constexpr std::size_t NUM_CLASSES = 10;
/// Size (in bytes) for each class: 8 through 4096.
static constexpr std::size_t SIZES[NUM_CLASSES] = {
    8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
};
/// Blocks available per size class (can differ per pool).
/// Right now it's just all same. But we can change!
static constexpr std::size_t BLOCKS_PER_POOL[NUM_CLASSES] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
/// Largest allocation the slab can do.
static constexpr std::size_t MAX_SIZE = 4096;

/// @brief Compute total backing storage at compile time.
static constexpr std::size_t total_storage_bytes() {
    std::size_t total = 0;
    for (std::size_t i = 0; i < NUM_CLASSES; ++i) {
        total += SIZES[i] * BLOCKS_PER_POOL[i];
    }
    return total;
}

/// Total backing storage = sum(SIZES[i] * BLOCKS_PER_POOL[i]).
static constexpr std::size_t TOTAL_BYTES = total_storage_bytes();

/// free-list node — stored inside each free block.
/// just stores the next node.
struct FreeBlock { FreeBlock* next; };

/// Static storage shared across all pools
static char storage[TOTAL_BYTES];
/// Per-class free-list heads (zero-initialised = all nullptr).
static FreeBlock* pools[NUM_CLASSES];
/// Bool check so init() runs exactly once
static bool ready = false;

/// @brief Find the smallest size class that fits `size` bytes.
static std::size_t class_for(std::size_t size) {
    for (std::size_t i = 0; i < NUM_CLASSES; ++i) {
        if (size <= SIZES[i]) return i;
    }
    // Couldn't find anything :(
    return NUM_CLASSES;
}

/// @brief Make storage into per-class pools and build free lists.
static void init() {
    char* cursor = storage;
    for (std::size_t c = 0; c < NUM_CLASSES; ++c) {
        std::size_t bsz = SIZES[c];
        for (std::size_t i = 0; i < BLOCKS_PER_POOL[c]; ++i) {
            FreeBlock* blk = reinterpret_cast<FreeBlock*>(cursor + i * bsz);
            blk->next = pools[c];
            pools[c] = blk;
        }
        cursor += bsz * BLOCKS_PER_POOL[c];
    }
    ready = true;
}

/// @brief Determine which pool a pointer belongs to from its address.
/// Each pool occupies a contiguous region in storage, so we walk
/// cumulative offsets to find the right class.
/// @return Pool index, or NUM_CLASSES if the pointer is outside storage.
static std::size_t pool_for_ptr(const void* ptr) {
    const char* p = static_cast<const char*>(ptr);
    if (p < storage || p >= storage + TOTAL_BYTES) return NUM_CLASSES;  // GCOVR_EXCL_BR_LINE
    std::size_t off = static_cast<std::size_t>(p - storage);
    std::size_t cursor = 0;
    for (std::size_t c = 0; c < NUM_CLASSES; ++c) {  // GCOVR_EXCL_BR_LINE
        std::size_t pool_bytes = SIZES[c] * BLOCKS_PER_POOL[c];
        if (off < cursor + pool_bytes) return c;  // GCOVR_EXCL_BR_LINE
        cursor += pool_bytes;
    }
    return NUM_CLASSES;  // GCOVR_EXCL_LINE
}

/// @brief Allocate `size` bytes from the appropriate pool.
/// If the best-fit pool is exhausted, tries the next larger pool(s).
/// @return Pointer to the block, or nullptr if oversized / all pools full.
static void* allocate(std::size_t size) {
    if (!ready) init();
    if (size == 0) size = 1;
    std::size_t idx = class_for(size);
    if (idx == NUM_CLASSES) return nullptr;

    // Try current class, then fall through to larger classes.
    for (std::size_t i = idx; i < NUM_CLASSES; ++i) {
        FreeBlock* blk = pools[i];
        if (blk) {
            pools[i] = blk->next;
            return blk;
        }
    }
    return nullptr;
}

/// @brief Return a block to the pool it actually came from.
/// Uses pointer address (not the caller's size) so that blocks
/// promoted to a larger pool during allocation are freed correctly.
static void deallocate(void* ptr, std::size_t) {
    if (!ptr) return;
    std::size_t idx = pool_for_ptr(ptr);
    if (idx == NUM_CLASSES) return;

    FreeBlock* blk = static_cast<FreeBlock*>(ptr);
    blk->next = pools[idx];
    pools[idx] = blk;
}

}  // namespace slab

#endif  // SRC_SLAB_SLAB_ALLOC_HPP_
