#include <cassert>
#include <cstdio>

#define ARENA_CAPACITY (16 * 1024)
#include "arena/arena_overrides.hpp"

struct Bar {
    int a, b, c;
};

int main() {
    std::puts("=== Arena new/delete Override Tests ===");

    // operator new + unsized delete
    auto* b = new Bar{1, 2, 3};
    assert(b->a == 1 && b->b == 2 && b->c == 3);
    assert(overrides_arena.used() > 0);
    delete b;

    // operator new[] + unsized delete[]
    auto* arr = new int[16];
    for (int i = 0; i < 16; ++i) arr[i] = i * 10;
    assert(arr[15] == 150);
    delete[] arr;

    // Explicitly call sized delete variants for coverage
    auto* b2 = new Bar{4, 5, 6};
    ::operator delete(static_cast<void*>(b2), sizeof(Bar));

    auto* arr2 = new int[4];
    arr2[0] = 99;
    ::operator delete[](static_cast<void*>(arr2), sizeof(int) * 4);

    // Test operator new throw on exhaustion
    overrides_arena.reset();
    overrides_arena.allocate(ARENA_CAPACITY);
    bool caught = false;
    try {
        void* p = ::operator new(1);
        (void)p;
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);

    // Test operator new[] throw on exhaustion
    bool caught_arr = false;
    try {
        void* p = ::operator new[](1);
        (void)p;
    } catch (const std::bad_alloc&) {
        caught_arr = true;
    }
    assert(caught_arr);

    overrides_arena.reset();
    assert(overrides_arena.used() == 0);

    std::puts("  PASS new/delete through arena");
    std::puts("=== All arena new/delete tests passed ===");
}
