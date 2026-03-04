#include <cassert>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#define ARENA_CAPACITY (32 * 1024)
#define HYBRID_THRESHOLD 256
#define NUM_ARENAS 3
#include "hybrid/hybrid_alloc.hpp"

static void test_small_goes_to_slab() {
    std::size_t before = hybrid::get_arena(0).used();
    void* p = hybrid::allocate(32);
    assert(p != nullptr);
    assert(hybrid::get_arena(0).used() == before);
    hybrid::deallocate(p, 32);
    std::puts("  PASS test_small_goes_to_slab");
}

static void test_large_goes_to_arena() {
    hybrid::reset_arenas();
    void* p = hybrid::allocate(512, 0);
    assert(p != nullptr);
    assert(hybrid::get_arena(0).used() == 512);
    hybrid::deallocate(p, 512);
    assert(hybrid::get_arena(0).used() == 512);
    std::puts("  PASS test_large_goes_to_arena");
}

static void test_threshold_boundary() {
    hybrid::reset_arenas();

    void* at = hybrid::allocate(HYBRID_THRESHOLD);
    assert(at != nullptr);
    assert(hybrid::get_arena(0).used() == 0);
    hybrid::deallocate(at, HYBRID_THRESHOLD);

    void* over = hybrid::allocate(HYBRID_THRESHOLD + 1, 0);
    assert(over != nullptr);
    assert(hybrid::get_arena(0).used() == HYBRID_THRESHOLD + 1);
    std::puts("  PASS test_threshold_boundary");
}

static void test_reset_arenas() {
    hybrid::reset_arenas();
    hybrid::allocate(1024, 0);
    hybrid::allocate(2048, 1);
    assert(hybrid::get_arena(0).used() == 1024);
    assert(hybrid::get_arena(1).used() == 2048);
    hybrid::reset_arenas();
    assert(hybrid::get_arena(0).used() == 0);
    assert(hybrid::get_arena(1).used() == 0);
    std::puts("  PASS test_reset_arenas");
}

static void test_multi_arena_isolation() {
    hybrid::reset_arenas();
    void* a0 = hybrid::allocate(512, 0);
    void* a1 = hybrid::allocate(512, 1);
    void* a2 = hybrid::allocate(512, 2);
    assert(a0 != nullptr && a1 != nullptr && a2 != nullptr);
    assert(hybrid::get_arena(0).used() == 512);
    assert(hybrid::get_arena(1).used() == 512);
    assert(hybrid::get_arena(2).used() == 512);
    // Each arena's pointer should be inside its own buffer.
    assert(hybrid::get_arena(0).contains(a0));
    assert(hybrid::get_arena(1).contains(a1));
    assert(hybrid::get_arena(2).contains(a2));
    assert(!hybrid::get_arena(0).contains(a1));
    std::puts("  PASS test_multi_arena_isolation");
}

static void test_slab_reuse_after_dealloc() {
    void* a = hybrid::allocate(16);
    hybrid::deallocate(a, 16);
    void* b = hybrid::allocate(16);
    assert(b == a);
    hybrid::deallocate(b, 16);
    std::puts("  PASS test_slab_reuse_after_dealloc");
}

static void test_zero_size_alloc() {
    void* p = hybrid::allocate(0);
    assert(p != nullptr);
    hybrid::deallocate(p, 0);
    std::puts("  PASS test_zero_size_alloc");
}

static void test_dealloc_nullptr() {
    hybrid::deallocate(nullptr, 8);
    hybrid::deallocate(nullptr, 512);
    std::puts("  PASS test_dealloc_nullptr");
}

static void test_stl_vector() {
    hybrid::reset_arenas();
    std::vector<int, hybrid::HybridSTL<int>> vec;
    for (int i = 0; i < 100; ++i) vec.push_back(i);
    assert(vec.size() == 100);
    assert(vec[99] == 99);
    std::puts("  PASS test_stl_vector");
}

static void test_stl_list_uses_slab() {
    hybrid::reset_arenas();
    std::list<int, hybrid::HybridSTL<int>> lst;
    for (int i = 0; i < 10; ++i) lst.push_back(i);
    assert(lst.size() == 10);
    assert(hybrid::get_arena(0).used() == 0);
    std::puts("  PASS test_stl_list_uses_slab");
}

static void test_stl_alloc_bad_alloc() {
    hybrid::reset_arenas();
    hybrid::get_arena(0).allocate(ARENA_CAPACITY);
    bool caught = false;
    try {
        hybrid::HybridSTL<char> a;
        a.allocate(4097);
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);
    hybrid::reset_arenas();
    std::puts("  PASS test_stl_alloc_bad_alloc");
}

static void test_slab_full_falls_back_to_arena() {
    hybrid::reset_arenas();
    // Dimension uses pool-0 count (all pools currently equal).
    void* held[slab::NUM_CLASSES][slab::BLOCKS_PER_POOL[0]];
    for (std::size_t c = 0; c < slab::NUM_CLASSES; ++c)
        for (std::size_t i = 0; i < slab::BLOCKS_PER_POOL[c]; ++i)
            held[c][i] = slab::allocate(slab::SIZES[c]);

    void* p = hybrid::allocate(8, 1);
    assert(p != nullptr);
    assert(hybrid::get_arena(1).used() == 8);
    hybrid::deallocate(p, 8);

    for (std::size_t c = 0; c < slab::NUM_CLASSES; ++c)
        for (std::size_t i = 0; i < slab::BLOCKS_PER_POOL[c]; ++i)
            slab::deallocate(held[c][i], slab::SIZES[c]);
    hybrid::reset_arenas();
    std::puts("  PASS test_slab_full_falls_back_to_arena");
}

int main() {
    std::puts("=== Hybrid Allocator Tests ===");
    test_small_goes_to_slab();
    test_large_goes_to_arena();
    test_threshold_boundary();
    test_reset_arenas();
    test_multi_arena_isolation();
    test_slab_reuse_after_dealloc();
    test_zero_size_alloc();
    test_dealloc_nullptr();
    test_slab_full_falls_back_to_arena();
    test_stl_vector();
    test_stl_list_uses_slab();
    test_stl_alloc_bad_alloc();
    std::puts("=== All hybrid tests passed ===");
}
