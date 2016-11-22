//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once
#include <pebble.h>

// #define DEBUG
// #define SHOW_BACKGROUND_BITMAP_IMAGE

#define BG_BITMAP_BG_COLOUR GColorBlack

#if PBL_DISPLAY_WIDTH == 200

#else
#define DIGIT_RECT_SIZE_W 24
#define DIGIT_RECT_SIZE_H 23
#define DIGIT_X_POS 36
#define DIGIT_Y_POS 42
#endif /* PBL_DISPLAY_WIDTH */

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

#if PBL_DISPLAY_WIDTH == 200

#else
#define HOUR_HAND_LENGTH 32
#define HOUR_HAND_THK 9
#define MIN_HAND_LENGTH 56
#define MIN_HAND_THK 5
#endif

#define CLOCKFACE_CENTER ( GPoint( PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2 ) )
#define HOUR_RECT ( GRect( PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2, PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2 - HOUR_HAND_LENGTH ) )
#define MIN_RECT ( GRect( PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2, PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2 - MIN_HAND_LENGTH ) )

#define ANIMATION_DELAY 0
#define ANIMATION_DURATION 2000

typedef struct {
  uint8_t digit;
  uint32_t colour;
  GRect home_rect;
  GRect current_rect;
} DIGIT_LAYER_DATA;

typedef struct {
  uint32_t colour;
  uint8_t stroke_width;
  uint16_t hole_radius;
  GRect layer_frame;
  GRect home_rect;
  GRect current_rect;
} HAND_LAYER_DATA;

#define NUM_DIGITS 12

BitmapLayer *clockface_layer;
Layer *digit_layer[ NUM_DIGITS ];
Layer *hour_layer;
Layer *min_layer;

tm tm_time;
