/**
 * @file   arenas.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Tue May 29 23:11:53 2018
 *
 * @brief  Arenas - Arena style memory allocator.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "arenas.h"


static ar_t      ar_new_common( ar_t ar, ar_size_t count, int resize );
static void      ar_allocate_bytes( ar_t ar );
static ar_size_t ar_alignment_pad( ar_t ar, ar_size_t alignment );
static int       ar_free_node( ar_t ar );
void             ar_void_assert( void );



ar_t ar_new( void )
{
    return ar_new_common( NULL, AR_DEFAULT_SIZE, 0 );
}


ar_t ar_init_sized( ar_t ar, ar_size_t count )
{
    return ar_new_common( ar, count, 0 );
}


ar_t ar_init_flexible( ar_t ar, ar_size_t count )
{
    return ar_new_common( ar, count, 1 );
}


void ar_use( ar_t ar, ar_d mem, ar_size_t size )
{
    ar->size = size;
    ar->used = 0;
    ar->data = mem;
}


void ar_destroy( ar_p arp )
{
    if ( *arp == NULL ) {
        return;
    }

    while ( ar_free_node( *arp ) )
        ;

    if ( ( *arp )->data ) {
        ar_free( ( *arp )->data );
    }

    ar_free( *arp );
    *arp = NULL;
}


void ar_free_pages( ar_t ar )
{
    while ( ar_free_node( ar ) )
        ;

    if ( ar->data ) {
        ar_free( ar->data );
    }

    ar->data = NULL;
    ar->used = 0;
}


ar_d ar_reserve( ar_t ar, ar_size_t size )
{
    ar_assert( ar->data != 0 );

    if ( ( ar->used + size ) > ar->size ) {

        if ( !ar->flex ) {

            return NULL;

        } else {

            ar_t prev;

            /* Create new Arenas Node. */
            prev = ar_malloc( sizeof( ar_s ) );
            *prev = *ar;
            ar->prev = prev;
            ar_allocate_bytes( ar );
            if ( ar->data == NULL ) {
                ar_assert( 0 ); // GCOV_EXCL_LINE
            }
        }
    }

    ar_d ret = ar->data + ar->used;
    ar->used += size;

    return ret;
}


ar_d ar_reserve_aligned( ar_t ar, ar_size_t size, ar_size_t alignment )
{
    ar->used += ar_alignment_pad( ar, alignment );
    return ar_reserve( ar, size );
}


void ar_release( ar_t ar, ar_d mem )
{
    ar_size_t size;

    ar_assert( ar->data != 0 );
    ar_assert( mem >= ar->data );

    if ( ar->used > 0 ) {
        size = ( ar->data + ar->used ) - mem;
        ar->used -= size;
    } else {
        ar_free_node( ar );
    }
}


void ar_release_all( ar_t ar )
{
    ar_assert( ar->data != 0 );

    while ( ar_free_node( ar ) )
        ;

    ar->used = 0;
}


ar_d ar_store( ar_t ar, ar_size_t size, ar_d data )
{
    if ( ( ar->used + size ) > ar->size ) {
        /* Ensure that all data is fitting, or nothing. */
        ar->used = ar->size;
    }

    ar_d seg;
    seg = ar_reserve( ar, size );

    if ( seg == NULL ) {
        return NULL;
    }

    memcpy( seg, data, size );

    return seg;
}


ar_d ar_store_aligned( ar_t ar, ar_size_t size, ar_d data, ar_size_t alignment )
{
    if ( ( ar->used + size + ar_alignment_pad( ar, alignment ) ) > ar->size ) {
        /* Ensure that all data is fitting, or nothing. */
        ar->used = ar->size;
    }

    ar_d seg;
    seg = ar_reserve_aligned( ar, size, alignment );

    if ( seg == NULL ) {
        return NULL;
    }

    memcpy( seg, data, size );

    return seg;
}


ar_size_t ar_page_size( void )
{
    return sysconf( _SC_PAGESIZE );
}


ar_size_t ar_reservation_size( ar_t ar )
{
    return ar->size * ar_pool_count( ar );
}


ar_size_t ar_pool_count( ar_t ar )
{
    ar_size_t cnt = 1;

    /* Roll back to previous Arenas Node. */
    while ( ar->prev ) {
        cnt++;
        ar = ar->prev;
    }

    return cnt;
}



/* ------------------------------------------------------------
 * Internal support:
 */

static ar_t ar_new_common( ar_t ar, ar_size_t count, int resize )
{
    if ( ar == NULL ) {
        ar = ar_malloc( sizeof( ar_s ) );
    }

    if ( count == 0 ) {
        count = 1;
    }

    ar->size = count * ar_page_size();
    ar_allocate_bytes( ar );

    if ( ar->data == NULL ) {
        ar_assert( 0 ); // GCOV_EXCL_LINE
    }

    ar->flex = resize;
    ar->prev = NULL;

    return ar;
}


static void ar_allocate_bytes( ar_t ar )
{
    ar->used = 0;
    if ( !posix_memalign( &ar->data, ar_page_size(), ar->size ) ) {
        memset( ar->data, 0, ar->size );
    } else {
        ar->data = NULL; // GCOV_EXCL_LINE
    }
}


static ar_size_t ar_alignment_pad( ar_t ar, ar_size_t alignment )
{
    ar_size_t addr;

    addr = (ar_size_t)ar->data + ar->used;

    ar_size_t mod;
    mod = addr % alignment;

    if ( mod == 0 ) {
        return 0;
    } else {
        return ( alignment - mod );
    }
}



static int ar_free_node( ar_t ar )
{
    ar_t rm;

    /* Roll back to previous Arenas Node. */
    if ( ar->prev ) {
        ar_free( ar->data );
        rm = ar->prev;
        *ar = *rm;
        ar_free( rm );
        return 1;
    } else {
        return 0;
    }
}



/**
 * Disabled (void) assertion.
 */
// GCOV_EXCL_START
void ar_void_assert( void ) {}
// GCOV_EXCL_STOP
