#include <cassert>
#include <cstdio>

#include "slab/slab_overrides.hpp"

struct Foo {
    int x;
    int y;
};

int main() {
    std::puts("=== Slab new/delete Override Tests ===");

    // operator new + unsized delete
    auto* f = new Foo{10, 20};
    assert(f->x == 10 && f->y == 20);
    ::operator delete(static_cast<void*>(f));

    // operator new[] + unsized delete[]
    auto* arr = new int[8];
    for (int i = 0; i < 8; ++i) arr[i] = i;
    assert(arr[7] == 7);
    ::operator delete[](static_cast<void*>(arr));

    // Explicitly call sized delete variants for coverage
    auto* f2 = new Foo{30, 40};
    ::operator delete(static_cast<void*>(f2), sizeof(Foo));

    auto* arr2 = new int[4];
    arr2[0] = 77;
    ::operator delete[](static_cast<void*>(arr2), sizeof(int) * 4);

    // Test operator new throw when slab cannot handle the size
    bool caught = false;
    try {
        // slab max is 4096; requesting more triggers nullptr + throw
        void* p = ::operator new(4097);
        (void)p;
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    assert(caught);

    // Test operator new[] throw
    bool caught_arr = false;
    try {
        void* p = ::operator new[](4097);
        (void)p;
    } catch (const std::bad_alloc&) {
        caught_arr = true;
    }
    assert(caught_arr);

    std::puts("  PASS new/delete through slab");
    std::puts("=== All slab new/delete tests passed ===");
}
