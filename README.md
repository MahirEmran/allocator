# Hybrid Allocator
Implementation of two allocators in C++. The two types are:
* Slab allocator ([https://en.wikipedia.org/wiki/Slab_allocation](https://en.wikipedia.org/wiki/Slab_allocation))
  * Essentially, great for objects (and other such items that are usually not long-living)
  * There are pools of various length, and it just tries to insert in the smallest block it can.
  * Can be shared across multiple programs iff they are run sequentially (parallel is another issue)
* Arena allocator ([https://protobuf.dev/reference/cpp/arenas/](https://protobuf.dev/reference/cpp/arenas/))
  * The link here is similar but not exact
  * It essentially just works by having some region of memory and just offsetting for each object.
  * Different programs can have different arenas of memory (I believe this is optional), but probably better practice to keep it separate. Especially if they run in parallel, this would help keep it concurrent.
    * I will implement this where each program's arena size can be different (but specified at runtime so it's still in static memory)
  
How slab will work, is that it will just be there. It's great for memory reusage, but not good for large buffers.
* Destructor behavior in C++ is essentially the same. It will free the block and add it back to the list

How arena would work, is that each program would have its own region, and just allocate.
* Once program is finished, the caller of that program should free the entire region.

Slab is generally bad when you have large items. Arena is bad for stuff being freed/allocated constantly (it doesn't actaully 'free' until end of program)


I have implemented 3 options here:
* Full slab allocator
* Full arena allocator
* Hybrid slab/arena (slab used for items <= 4096 bytes, arena used for everything else)

Each of these contain code overriding the C++ new/delete/new[]/delete[] and STL container template classes, as appropriate.