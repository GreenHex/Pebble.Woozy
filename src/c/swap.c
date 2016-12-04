//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//
// Source: http://www.geeksforgeeks.org/shuffle-a-given-array/
//

#include <pebble.h>
#include "swap.h"

static void swap( int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
 
void randomize( int arr[], int n ) {
    srand( time( NULL ) );
    for ( int i = n-1; i > 0; i-- ) {
        int j = rand() % ( i + 1 );
        swap( &arr[i], &arr[j] );
    }
}
