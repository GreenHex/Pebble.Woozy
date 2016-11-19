//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once
#include <pebble.h>
#include "global.h"

#if PBL_DISPLAY_WIDTH == 200

#else
#define DIGIT_RECT_SIZE_W 24
#define DIGIT_RECT_SIZE_H 21
#define DIGIT_X_POS 36
#define DIGIT_Y_POS 42
#endif

#define DIGIT_TXT_VERT_ADJ 2

static const GPathInfo DIGIT_LOCATIONS = {
  12, (GPoint []) {
    { PBL_DISPLAY_WIDTH/2 + DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, 0 }, // 1
    { PBL_DISPLAY_WIDTH - DIGIT_RECT_SIZE_W, PBL_DISPLAY_HEIGHT/2 - DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2 }, // 2
    { PBL_DISPLAY_WIDTH - DIGIT_RECT_SIZE_W, PBL_DISPLAY_HEIGHT/2 - DIGIT_RECT_SIZE_H/2 }, // 3
    { PBL_DISPLAY_WIDTH - DIGIT_RECT_SIZE_W, PBL_DISPLAY_HEIGHT/2 + DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2  }, // 4
    { PBL_DISPLAY_WIDTH/2 + DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, PBL_DISPLAY_HEIGHT - DIGIT_RECT_SIZE_H }, // 5
    { PBL_DISPLAY_WIDTH/2 - DIGIT_RECT_SIZE_W/2, PBL_DISPLAY_HEIGHT - DIGIT_RECT_SIZE_H }, // 6
    { PBL_DISPLAY_WIDTH/2 - DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, PBL_DISPLAY_HEIGHT - DIGIT_RECT_SIZE_H }, // 7
    { 0, PBL_DISPLAY_HEIGHT/2 + DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2 }, // 8
    { 0, PBL_DISPLAY_HEIGHT/2 - DIGIT_RECT_SIZE_H/2 }, // 9
    { 0,  PBL_DISPLAY_HEIGHT/2 - DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2 }, // 10
    { PBL_DISPLAY_WIDTH/2 - DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, 0 }, // 11
    { PBL_DISPLAY_WIDTH/2 - DIGIT_RECT_SIZE_W/2, 0 }, // 12
  }
};

typedef struct {
  uint8_t digit;
  GRect home_frame;
  GRect current_frame;
} DIGIT_LAYER_DATA;

bool is_X_in_range( int a, int b, int x );
void clock_init( Window* window );
void clock_deinit( void );
