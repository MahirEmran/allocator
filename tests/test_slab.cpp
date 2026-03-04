#include <cassert>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#include "slab/slab_alloc.hpp"

static void test_basic_alloc_dealloc() {
    void* p = slab::allocate(10);
    assert(p != nullptr);
    std::memset(p, 0xAB, 10);
    slab::deallocate(p, 10);

    void* p2 = slab::allocate(10);
    assert(p2 == p);
    slab::deallocate(p2, 10);

    std::puts("  PASS test_basic_alloc_dealloc");
}

static void test_all_size_classes() {
    for (std::size_t i = 0; i < slab::NUM_CLASSES; ++i) {
        void* p = slab::allocate(slab::SIZES[i]);
        assert(p != nullptr);
        std::memset(p, 0, slab::SIZES[i]);
        slab::deallocate(p, slab::SIZES[i]);
    }
    std::puts("  PASS test_all_size_classes");
}

static void test_oversized_returns_null() {
    void* p = slab::allocate(slab::MAX_SIZE + 1);
    assert(p == nullptr);
    std::puts("  PASS test_oversized_returns_null");
}

static void test_pool_exhaustion() {
    // Use the largest class so there is no next-pool fallback.
    static constexpr std::size_t N = slab::BLOCKS_PER_POOL[slab::NUM_CLASSES - 1];
    void* ptrs[N];

    for (std::size_t i = 0; i < N; ++i) {
        ptrs[i] = slab::allocate(slab::MAX_SIZE);
        assert(ptrs[i] != nullptr);
    }
    void* extra = slab::allocate(slab::MAX_SIZE);
    assert(extra == nullptr);

    for (std::size_t i = 0; i < N; ++i)
        slab::deallocate(ptrs[i], slab::MAX_SIZE);

    void* recovered = slab::allocate(slab::MAX_SIZE);
    assert(recovered != nullptr);
    slab::deallocate(recovered, slab::MAX_SIZE);

    std::puts("  PASS test_pool_exhaustion");
}

static void test_zero_size_alloc() {
    // size==0 is promoted to 1 internally
    void* p = slab::allocate(0);
    assert(p != nullptr);
    slab::deallocate(p, 0);
    std::puts("  PASS test_zero_size_alloc");
}

static void test_dealloc_nullptr() {
    slab::deallocate(nullptr, 8);
    std::puts("  PASS test_dealloc_nullptr");
}

static void test_dealloc_outside_storage() {
    // deallocate with a pointer not from slab storage is silently ignored
    int stack_var = 42;
    slab::deallocate(&stack_var, 8);
    std::puts("  PASS test_dealloc_outside_storage");
}

static void test_stl_vector() {
    std::vector<int, slab::SlabSTL<int>> vec;
    for (int i = 0; i < 20; ++i) vec.push_back(i);
    assert(vec.size() == 20);
    assert(vec[0] == 0);
    assert(vec[19] == 19);
    std::puts("  PASS test_stl_vector");
}

static void test_stl_list() {
    std::list<int, slab::SlabSTL<int>> lst;
    for (int i = 0; i < 10; ++i) lst.push_back(i * 10);
    assert(lst.size() == 10);
    assert(lst.front() == 0);
    assert(lst.back() == 90);
    std::puts("  PASS test_stl_list");
}

static void test_stl_alloc_bad_alloc() {
    // Exhaust the largest pool (no next-pool fallback)
    static constexpr std::size_t N = slab::BLOCKS_PER_POOL[slab::NUM_CLASSES - 1];
    void* ptrs[N];
    for (std::size_t i = 0; i < N; ++i) {
        ptrs[i] = slab::allocate(slab::MAX_SIZE);
    }

    bool caught = false;
    try {
        slab::SlabSTL<char> a;
        a.allocate(slab::MAX_SIZE);
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);

    for (std::size_t i = 0; i < N; ++i)
        slab::deallocate(ptrs[i], slab::MAX_SIZE);

    std::puts("  PASS test_stl_alloc_bad_alloc");
}

int main() {
    std::puts("=== Slab Allocator Tests ===");
    test_basic_alloc_dealloc();
    test_all_size_classes();
    test_oversized_returns_null();
    test_pool_exhaustion();
    test_zero_size_alloc();
    test_dealloc_nullptr();
    test_dealloc_outside_storage();
    test_stl_vector();
    test_stl_list();
    test_stl_alloc_bad_alloc();
    std::puts("=== All slab tests passed ===");
}
