# Arenas - Arena style memory allocator

Arenas is a OS memory page based allocator. It is targeted for
mid-lifetime objects. Mid-lifetime objects live over multiple function
executions, and hence can't be allocated from stack. On the other hand
they don't live the whole program execution, and thus normal heap
allocation might be sub-optimal and involve too much overhead,
especially if the allocation and deallocation occurs frequently.

Arenas contains a continuous junk of memory which is a multiple of the
OS memory page size, typically 4kB. Clients can allocate memory from
Arenas piece by piece, and the allocations will be continuous in
memory. This promotes memory locality and hence improves cache
efficiency.

Common use case for Arenas is to provide a scratch pad type of
area. Allocations are performed, but they are not released. The
allocations accumulate, until at some point in the program all
allocations are released. For example Slinky-library strings can be
allocated with `sl_use` from Arenas. If Slinky grows over the
allocated size, Slinky will changed to heap allocation, and the Arenas
side allocation becomes redundant. Other clients can still reserve
memory from Arenas.

Another use case for Arenas is to use it as stack allocator. The
reservations should be released in the reverse order. The client must
store all reservations addresses, in order to ensure correct release,
and prevent data corruption with other clients. A typical usage
pattern is where different phases of a function reserve and release
memory in turns, i.e. the stack height is only item at a time.

Arenas consists of OS memory page based memory pool and a
Descriptor. The memory pool is aligned memory and the Descriptor is a
separate allocation, which includes: pointer to the pool, pool size,
and the usage count for memory reservations.

Arenas can be created in the default setting performing:

    ar_t ar = ar_new();

The created Arenas has size of one page, and the returned Arenas
Descriptor is heap allocated, and hence should be freed by some means
(see below). Arenas has fixed size and it returns `NULL` for
reservations when all memory has been used.

Another way of creating the Arenas is to specify the size as well as
expressing whether the Descriptor should be only initialized or
additionally allocated as well.

    ar_s ard;
    ar_new_sized( &ard, 2 );

The example above will initialize the stack allocated Descriptor and
it will be setup to contain two pages of memory. Alternatively we
could request to allocate the Descriptor as well.

    ar = ar_new_size( NULL, 2 );

When `NULL` is supplied to `ar_new_size`, it will allocate the
Descriptor along with the memory pool.

Arenas can be created also in flexible mode, where additional memory
pages are allocated as needed.

    ar = ar_new_flexible( &ard, 4 );

In flexible mode more memory is allocated, but the additional pages
are not continous compared to the previous allocation. However, all
fresh memory is continous.

Arenas with stack allocated Descriptor is destroyed using:

    ar_free_pages( &ard );

and heap allocated Descriptor is destroyed using:

    ar_destroy( &ar );

Note that reference to Arenas is supplied and that the referenced
pointer will be nulled.

An existing memory pool can be applied to Descriptor using:

    ar_use( ar, mem, size );

Pool address (`mem`) is supplied along with the size (`size`) of the
pool.

Memory is allocated from Arenas pool using `ar_reserve`.

    mem = ar_reserve( ar, 128 );

This invocation will reserve 128 bytes from memory pool and return the
base address of the reservation. Arenas will return consecutive
segments of memory from the pool, by default. If client requires
aligned memory reservations, the `ar_reserve_aligned` function can be
used.

    mem = ar_reserve_aligned( ar, 128, 512 );

Here the reserved segment is again 128 bytes, but it is aligned to 512
byte boundary. Alignment will consume typically some extra memory from
the pool.

Memory is release with `ar_release` function. `ar_release` is given
the base address of the reservation that was originally returned by
`ar_reserve` (or `ar_reserve_aligned`).

    ar_release( ar, mem );

At some point during program execution it might be convenient to clear
all reservations and start over. Obviously this means that none of the
reservations are active, and they should be redundant.

    ar_release_all( ar );

`ar_release_all` removes all reservations, but does not free the
memory pool, and hence reservations can be done again from fresh
pool. In summary there are 4 levels of memory release:

* `ar_release`: release last allocation.

* `ar_release_all`: release all allocations.

* `ar_free_pages`: release all allocations and release the pool, but
  not the Descriptor (that lives in stack).

* `ar_destroy`: release all of the above and also the heap allocated
  Descriptor.

Arenas can be used to store data.

    mem = ar_store( ar, strlen( hello )+1, hello );

There is also aligned version for store,
i.e. `ar_store_aligned`. `ar_store` is the same as `ar_reserve` except
that in addition to reservation the content is initialized as
well. `ar_store` returns address of reservation (and copy of data) on
success. If the stored data does not completely fit into available
storage, either `NULL` is returned (in fixed mode) and nothing is
stored, or new pages are allocated (in flexible mode) and data is
copied to newly allocated pages.

By default Arenas library uses malloc and friends to do heap
allocations. If you define ARENAS_MEM_API, you can use your own memory
allocation functions.

Custom memory function prototypes:
    void* gr_malloc ( size_t size );
    void  gr_free   ( void*  ptr  );
    void* gr_realloc( void*  ptr, size_t size );


See Doxygen docs and `arenas.h` for details about Arenas API. Also
consult the test directory for usage examples.


## Arenas API documentation

See Doxygen documentation. Documentation can be created with:

    shell> doxygen .doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building

Ceedling based flow is in use:

    shell> ceedling

Testing:

    shell> ceedling test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Arenas uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new arenas

in the directory above Arenas. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.
