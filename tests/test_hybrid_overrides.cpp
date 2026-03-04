#include <cassert>
#include <cstdio>

#define ARENA_CAPACITY (32 * 1024)
#define HYBRID_THRESHOLD 256
#define NUM_ARENAS 2
#include "hybrid/hybrid_overrides.hpp"

struct Small {
    char data[16];
};

struct Large {
    char data[1024];
};

int main() {
    std::puts("=== Hybrid new/delete Override Tests ===");

    // Small allocation (goes to slab)
    auto* s = new Small{};
    assert(hybrid::get_arena(0).used() == 0);
    delete s;

    // Large allocation (goes to arena 0 by default)
    auto* l = new Large{};
    assert(hybrid::get_arena(0).used() >= sizeof(Large));
    delete l;

    // Array allocation
    auto* arr = new int[4];
    arr[0] = 42;
    assert(arr[0] == 42);
    delete[] arr;

    // Explicitly call sized delete variants for coverage
    auto* s2 = new Small{};
    ::operator delete(static_cast<void*>(s2), sizeof(Small));

    auto* arr2 = new int[4];
    arr2[0] = 55;
    ::operator delete[](static_cast<void*>(arr2), sizeof(int) * 4);

    // Test operator new throw on arena exhaustion (large alloc)
    hybrid::reset_arenas();
    hybrid::get_arena(0).allocate(ARENA_CAPACITY);
    bool caught = false;
    try {
        // size > HYBRID_THRESHOLD routes to arena 0 which is full
        void* p = ::operator new(HYBRID_THRESHOLD + 1);
        (void)p;
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);

    // Test operator new[] throw
    bool caught_arr = false;
    try {
        void* p = ::operator new[](HYBRID_THRESHOLD + 1);
        (void)p;
    } catch (const std::bad_alloc&) {
        caught_arr = true;
    }
    assert(caught_arr);

    hybrid::reset_arenas();
    assert(hybrid::get_arena(0).used() == 0);

    std::puts("  PASS new/delete through hybrid");
    std::puts("=== All hybrid new/delete tests passed ===");
}
