#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#define ARENA_CAPACITY (8 * 1024)
#include "arena/arena_alloc.hpp"

static arena::Arena a;

static void test_basic_alloc() {
    a.reset();
    void* p = a.allocate(64);
    assert(p != nullptr);
    std::memset(p, 0xCD, 64);
    assert(a.used() == 64);
    assert(a.remaining() == 8 * 1024 - 64);
    std::puts("  PASS test_basic_alloc");
}

static void test_sequential_bumps() {
    a.reset();
    void* p1 = a.allocate(100);
    void* p2 = a.allocate(200);
    void* p3 = a.allocate(300);
    assert(p1 != nullptr && p2 != nullptr && p3 != nullptr);
    assert(static_cast<char*>(p2) == static_cast<char*>(p1) + 100);
    assert(static_cast<char*>(p3) == static_cast<char*>(p2) + 200);
    assert(a.used() == 600);
    std::puts("  PASS test_sequential_bumps");
}

static void test_deallocate_is_noop() {
    a.reset();
    void* p = a.allocate(128);
    std::size_t used_before = a.used();
    a.deallocate(p, 128);
    assert(a.used() == used_before);
    std::puts("  PASS test_deallocate_is_noop");
}

static void test_reset_wipes() {
    a.reset();
    a.allocate(1000);
    a.allocate(2000);
    assert(a.used() == 3000);
    a.reset();
    assert(a.used() == 0);
    assert(a.remaining() == 8 * 1024);
    std::puts("  PASS test_reset_wipes");
}

static void test_exhaustion() {
    a.reset();
    void* p = a.allocate(8 * 1024);
    assert(p != nullptr);
    assert(a.remaining() == 0);
    void* fail = a.allocate(1);
    assert(fail == nullptr);
    std::puts("  PASS test_exhaustion");
}

static void test_stl_vector() {
    a.reset();
    arena::ArenaSTL<int> alloc(a);
    std::vector<int, arena::ArenaSTL<int>> vec(alloc);
    for (int i = 0; i < 50; ++i) vec.push_back(i * i);
    assert(vec.size() == 50);
    assert(vec[0] == 0);
    assert(vec[49] == 2401);
    std::puts("  PASS test_stl_vector");
}

static void test_stl_alloc_bad_alloc() {
    a.reset();
    a.allocate(ARENA_CAPACITY);
    bool caught = false;
    try {
        arena::ArenaSTL<int> alloc(a);
        alloc.allocate(1);
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);
    a.reset();
    std::puts("  PASS test_stl_alloc_bad_alloc");
}

static void test_capacity_and_contains() {
    a.reset();
    assert(arena::Arena::capacity() == ARENA_CAPACITY);
    void* p = a.allocate(16);
    assert(a.contains(p));
    int stack_var = 0;
    assert(!a.contains(&stack_var));
    std::puts("  PASS test_capacity_and_contains");
}

int main() {
    std::puts("=== Arena Allocator Tests ===");
    test_basic_alloc();
    test_sequential_bumps();
    test_deallocate_is_noop();
    test_reset_wipes();
    test_exhaustion();
    test_stl_vector();
    test_stl_alloc_bad_alloc();
    test_capacity_and_contains();
    std::puts("=== All arena tests passed ===");
}
