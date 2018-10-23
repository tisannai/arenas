#include "unity.h"
#include "arenas.h"
#include <string.h>
#include <unistd.h>

void test_basics( void )
{
    ar_s ard;
    ar_t ar;
    ar_d mem;

    ar = &ard;

    ar_new_sized( ar, 0 );
    mem = ar_reserve( ar, 128 );
    TEST_ASSERT_EQUAL( 128, ar->used );
    mem = ar_reserve( ar, 128 );
    TEST_ASSERT_EQUAL( 256, ar->used );
    ar_free_pages( ar );

    ar = ar_new();
    mem = ar_reserve( ar, 128 );
    TEST_ASSERT_EQUAL( 128, ar->used );
    mem = ar_reserve( ar, 128 );
    TEST_ASSERT_EQUAL( 256, ar->used );
    ar_destroy( &ar );

    ar_destroy( &ar );


    ar = &ard;
    ar_new_sized( ar, 1 );
    mem = ar_reserve( ar, ar->size );
    TEST_ASSERT_TRUE( mem != NULL );
    mem = ar_reserve( ar, 1 );
    TEST_ASSERT_TRUE( mem == NULL );
    mem = ar_store( ar, 8, ar );
    TEST_ASSERT_TRUE( mem == NULL );
    mem = ar_store_aligned( ar, 8, ar, 16 );
    TEST_ASSERT_TRUE( mem == NULL );
    ar_release_all( ar );

    mem = ar_reserve( ar, ar->size / 2 );
    TEST_ASSERT_TRUE( mem != NULL );
    mem = ar_reserve( ar, ar->size / 2 );
    TEST_ASSERT_TRUE( mem != NULL );
    ar_release( ar, mem );
    ar_release( ar, ar->data );
    ar_release_all( ar );

    mem = ar_reserve( ar, 8 );
    TEST_ASSERT_TRUE( mem == ar->data );
    mem = ar_reserve_aligned( ar, 8, 128 );
    TEST_ASSERT_TRUE( mem == ( ar->data + 128 ) );
    mem = ar_reserve_aligned( ar, 128, 128 );
    TEST_ASSERT_TRUE( mem == ( ar->data + 2*128 ) );

    TEST_ASSERT_EQUAL( 3*128, ar->used );

    ar_release( ar, mem );
    ar_release( ar, ar->data+128 );
    ar_release( ar, ar->data );
    TEST_ASSERT_EQUAL( 0, ar->used );
    ar_release( ar, ar->data );
    
    ar_s ar2;

    ar_use( &ar2, ar->data, ar->size );
    mem = ar_reserve( ar, ar->size / 2 );
    TEST_ASSERT_TRUE( mem != NULL );
    mem = ar_reserve( ar, ar->size / 2 );
    TEST_ASSERT_TRUE( mem != NULL );

    ar_free_pages( ar );
}


void test_store( void )
{
    ar_s ard;
    ar_t ar;
    ar_d mem;

    int pagesize = sysconf( _SC_PAGESIZE );
    uint8_t data[ pagesize ];

    for ( int i = 0; i < pagesize; i++ ) {
        data[ i ] = (uint8_t) i;
    }

    ar = &ard;
    ar_new_flexible( ar, 1 );

    /* Non-aligned storage. */
    mem = ar_store( ar, 128, data );
    TEST_ASSERT_EQUAL( 128, ar->used );
    TEST_ASSERT_TRUE( mem == ar->data );

    mem = ar_store( ar, pagesize, data );
    TEST_ASSERT_EQUAL( pagesize, ar->used );
    
    mem = ar_store_aligned( ar, 128, data, pagesize );
    TEST_ASSERT_EQUAL( 128, ar->used );
    TEST_ASSERT_TRUE( mem == ar->data );

    ar_free_pages( ar );
}
