#ifndef ARENAS_H
#define ARENAS_H

/**
 * @file   arenas.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Tue May 29 23:11:53 2018
 *
 * @brief  Arenas - Arena style memory allocator.
 *
 */

#include <stdlib.h>
#include <stdint.h>



#ifndef ARENAS_NO_ASSERT
#include <assert.h>
/** Default assert. */
#define ar_assert assert
#else
/** Disabled assertion. */
#define ar_assert( cond ) (void)( ( cond ) || ar_void_assert() )
#endif


#ifndef AR_DEFAULT_SIZE
/** Default size for Arenas. */
#define AR_DEFAULT_SIZE 1
#endif


/** Size type. */
typedef uint64_t ar_size_t;

/** Position type. */
typedef int64_t ar_pos_t;

/** Data pointer type. */
typedef void* ar_d;


/**
 * Arenas struct.
 */
struct ar_struct_s
{
    ar_size_t           size; /**< Reservation size for data pool. */
    ar_size_t           used; /**< Used count for data. */
    ar_d                data; /**< Pointer to data pool. */
    struct ar_struct_s* prev; /**< Pointer to previous pool. */
    ar_size_t           flex; /**< Flexible pooling. */
};
typedef struct ar_struct_s ar_s; /**< Arenas struct. */
typedef ar_s*              ar_t; /**< Arenas pointer. */
typedef ar_t*              ar_p; /**< Arenas pointer reference. */


/* clang-format off */

#ifdef ARENAS_USE_MEM_API

/*
 * ARENAS_USE_MEM_API allows to use custom memory allocation functions,
 * instead of the default: ar_malloc, ar_free, ar_realloc.
 *
 * If ARENAS_USE_MEM_API is used, the user must provide implementation for
 * the above functions and they must be compatible with malloc
 * etc. Also Arenas assumes that ar_malloc sets all new memory to
 * zero.
 *
 * Additionally user should compile the library by own means.
 */

extern void* ar_malloc( size_t size );
extern void  ar_free( void* ptr );
extern void* ar_realloc( void* ptr, size_t size );

#else /* ARENAS_USE_MEM_API */


#    if SIXTEN_USE_MEM_API == 1

#        define gr_malloc  st_alloc
#        define gr_free    st_del
#        define gr_realloc st_realloc


#    else /* SIXTEN_USE_MEM_API == 1 */

/* Default to common memory management functions. */

/** Reserve memory. */
// #        define ar_malloc( size ) calloc( 1, size )
#        define ar_malloc( size ) calloc( 1, (size) )

/** Release memory. */
#        define ar_free free

/** Re-reserve memory. */
#        define ar_realloc realloc

#    endif /* SIXTEN_USE_MEM_API == 1 */

#endif /* ARENAS_USE_MEM_API */

/* clang-format on */


/**
 * Create Arenas with fixed default size (AR_DEFAULT_SIZE).
 *
 * @return Arenas.
 */
ar_t ar_new( void );


/**
 * Initialize Arenas to given size.
 *
 * Size of 0 is automatically converted to 1.
 *
 * @param ar     Arenas.
 * @param count  Page count.
 *
 * @return Arenas.
 */
ar_t ar_init_sized( ar_t ar, ar_size_t count );


/**
 * Initialize Arenas to given size and enable resizing mode.
 *
 * @param ar     Arenas.
 * @param count  Page count.
 *
 * @return Arenas.
 */
ar_t ar_init_flexible( ar_t ar, ar_size_t count );


/**
 * Use existing memory pool with Arenas Descriptor.
 *
 * @param ar   Arenas (Descriptor).
 * @param mem  Memory pool to use.
 * @param size Size of the memory pool.
 */
void ar_use( ar_t ar, ar_d mem, ar_size_t size );


/**
 * Destroy Arenas and Arenas Descriptor.
 *
 * @param arp Arenas reference.
 */
void ar_destroy( ar_p arp );


/**
 * Release Arenas allocation.
 *
 * Use ar_free_pages() instead of ar_destroy() if Arenas Descriptor is
 * stack allocated.
 *
 * @param ar
 */
void ar_free_pages( ar_t ar );


/**
 * Reserve a junk of memory from Arenas.
 *
 * @param ar   Arenas.
 * @param size Reservation size.
 *
 * @return Reservation start address.
 */
ar_d ar_reserve( ar_t ar, ar_size_t size );


/**
 * Reserve a junk of memory from Arenas with desired data alignment.
 *
 * @param ar        Arenas.
 * @param size      Reservation size.
 * @param alignment Alignment.
 *
 * @return Reservation start address.
 */
ar_d ar_reserve_aligned( ar_t ar, ar_size_t size, ar_size_t alignment );


/**
 * Release memory.
 *
 * NOTE: Release can only be used if all memory releases are performed
 * in order.
 *
 * @param ar  Arenas.
 * @param mem Reservation start address.
 */
void ar_release( ar_t ar, ar_d mem );


/**
 * Release all reservations.
 *
 * @param ar Arenas.
 */
void ar_release_all( ar_t ar );


/**
 * Store data to Arenas.
 *
 * Allocation is performed and data is copied to Arenas storage.
 *
 * @param ar   Arenas.
 * @param size Data size.
 * @param data Data.
 *
 * @return Storage address.
 */
ar_d ar_store( ar_t ar, ar_size_t size, ar_d data );


/**
 * Store data to Arenas with alignment.
 *
 * Allocation is performed and data is copied to Arenas storage.
 *
 * @param ar         Arenas.
 * @param size       Data size.
 * @param data       Data.
 * @param alignment  Alignment.
 *
 * @return Storage address.
 */
ar_d ar_store_aligned( ar_t ar, ar_size_t size, ar_d data, ar_size_t alignment );


/**
 * Return page size (of the system)
 *
 * @return Page size.
 */
ar_size_t ar_page_size( void );


/**
 * Return total reservation.
 *
 * @param ar Arenas.
 *
 * @return Total reservation.
 */
ar_size_t ar_reservation_size( ar_t ar );


/**
 * Return pool count.
 *
 * @param ar Arenas.
 *
 * @return Pool count;
 */
ar_size_t ar_pool_count( ar_t ar );


#endif
