#pragma once
#include <pebble.h>
#include "global.h"

///////
#if PBL_DISPLAY_WIDTH == 200
/////// 200 x 228

#define MIN_HAND_LENGTH 70
#define MIN_HAND_WIDTH 13
#define SEC_DOT_DIST 89
#define SEC_DOT_RADIUS 4

///////
#else
/////// 144 x 168

#define MIN_HAND_LENGTH 53
#define MIN_HAND_WIDTH 9
#define SEC_DOT_DIST 65
#define SEC_DOT_RADIUS 2

///////
#endif
///////

bool is_X_in_range( int a, int b, int x );
void draw_clock( void );
void clock_init( Window* window );
void clock_deinit( void );
