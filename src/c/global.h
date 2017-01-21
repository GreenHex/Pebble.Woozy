//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once
#include <pebble.h>

// #define DEBUG
// #define SHOW_BACKGROUND_BITMAP_IMAGE

#define BG_BITMAP_BG_COLOUR GColorBlack

#if PBL_DISPLAY_WIDTH == 200

#define DIGIT_RECT_SIZE_W 30
#define DIGIT_RECT_SIZE_H 32
#define DIGIT_X_POS 56
#define DIGIT_Y_POS 54

#define HOUR_HAND_LENGTH 48
#define HOUR_HAND_THK 13
#define MIN_HAND_LENGTH 72
#define MIN_HAND_THK 9

#else

#define DIGIT_RECT_SIZE_W 24
#define DIGIT_RECT_SIZE_H 22
#define DIGIT_X_POS 42
#define DIGIT_Y_POS 38

#define HOUR_HAND_LENGTH 30
#define HOUR_HAND_THK 9
#define MIN_HAND_LENGTH 50
#define MIN_HAND_THK 5

#endif /* PBL_DISPLAY_WIDTH */

#define DIGIT_TOP_BOTTOM_GAP 2
#define DIGIT_TXT_VERT_ADJ 3

static const GPathInfo DIGIT_LOCATIONS = {
  12, (GPoint []) {
    { PBL_DISPLAY_WIDTH/2 + DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, DIGIT_TOP_BOTTOM_GAP }, // 1
    { PBL_DISPLAY_WIDTH - DIGIT_RECT_SIZE_W, PBL_DISPLAY_HEIGHT/2 - DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2 }, // 2
    { PBL_DISPLAY_WIDTH - DIGIT_RECT_SIZE_W, PBL_DISPLAY_HEIGHT/2 - DIGIT_RECT_SIZE_H/2 }, // 3
    { PBL_DISPLAY_WIDTH - DIGIT_RECT_SIZE_W, PBL_DISPLAY_HEIGHT/2 + DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2  }, // 4
    { PBL_DISPLAY_WIDTH/2 + DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, PBL_DISPLAY_HEIGHT - DIGIT_RECT_SIZE_H - DIGIT_TOP_BOTTOM_GAP }, // 5
    { PBL_DISPLAY_WIDTH/2 - DIGIT_RECT_SIZE_W/2, PBL_DISPLAY_HEIGHT - DIGIT_RECT_SIZE_H - DIGIT_TOP_BOTTOM_GAP }, // 6
    { PBL_DISPLAY_WIDTH/2 - DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, PBL_DISPLAY_HEIGHT - DIGIT_RECT_SIZE_H - DIGIT_TOP_BOTTOM_GAP }, // 7
    { 0, PBL_DISPLAY_HEIGHT/2 + DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2 }, // 8
    { 0, PBL_DISPLAY_HEIGHT/2 - DIGIT_RECT_SIZE_H/2 }, // 9
    { 0,  PBL_DISPLAY_HEIGHT/2 - DIGIT_Y_POS - DIGIT_RECT_SIZE_H/2 }, // 10
    { PBL_DISPLAY_WIDTH/2 - DIGIT_X_POS - DIGIT_RECT_SIZE_W/2, DIGIT_TOP_BOTTOM_GAP }, // 11
    { PBL_DISPLAY_WIDTH/2 - DIGIT_RECT_SIZE_W/2, DIGIT_TOP_BOTTOM_GAP }, // 12
  }
};

#define CLOCKFACE_CENTER ( GPoint( PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2 ) )
#define HOUR_RECT ( GRect( PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2, PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2 - HOUR_HAND_LENGTH ) )
#define MIN_RECT ( GRect( PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2, PBL_DISPLAY_WIDTH/2, PBL_DISPLAY_HEIGHT/2 - MIN_HAND_LENGTH ) )

#if PBL_DISPLAY_WIDTH == 200
#define DAY_RECT_POS_X 36
#define DAY_RECT_POS_Y 132
#else
#define DAY_RECT_POS_X 24
#define DAY_RECT_POS_Y 105
#endif
#define DAY_RECT_SIZE_W ( DIGIT_RECT_SIZE_W * 3 )
#define DAY_RECT_SIZE_H DIGIT_RECT_SIZE_H
#define DAY_RECT ( GRect( DAY_RECT_POS_X, DAY_RECT_POS_Y, DAY_RECT_SIZE_W, DAY_RECT_SIZE_H ) )

#if PBL_DISPLAY_WIDTH == 200
#define DATE_RECT_POS_X 96
#define DATE_RECT_POS_Y DAY_RECT_POS_Y
#else
#define DATE_RECT_POS_X 74
#define DATE_RECT_POS_Y DAY_RECT_POS_Y
#endif
#define DATE_RECT_SIZE_W ( DIGIT_RECT_SIZE_W * 2 )
#define DATE_RECT_SIZE_H DIGIT_RECT_SIZE_H
#define DATE_RECT ( GRect( DATE_RECT_POS_X, DATE_RECT_POS_Y, DATE_RECT_SIZE_W, DATE_RECT_SIZE_H ) )

#define SNOOZE_LAYER_FRAME ( GRect( PBL_DISPLAY_WIDTH / 2 - 20 / 2, 36, 20, 20 ) )

#define ICON_OOPS_SIZE_X 64
#define ICON_OOPS_SIZE_Y 64
#define ICON_OOPS_LOC_X ( PBL_DISPLAY_WIDTH/2 - ICON_OOPS_SIZE_X/2 )
#define ICON_OOPS_LOC_Y ( PBL_DISPLAY_HEIGHT/2 - ICON_OOPS_SIZE_Y/2 + 6 )
#define OOPS_RECT ( GRect( ICON_OOPS_LOC_X, ICON_OOPS_LOC_Y, ICON_OOPS_SIZE_X, ICON_OOPS_SIZE_Y ) )

#define SHOW_TIME_TIMER_TIMEOUT_MS ( 10 * 1000 )

typedef struct {
  uint8_t digit;
  uint32_t colour;
  GTextAlignment text_alignment;
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
Layer *snooze_layer;
Layer *digit_layer[ NUM_DIGITS ];
Layer *hour_layer;
Layer *min_layer;
Layer *day_layer;
Layer *date_layer;
BitmapLayer *oops_layer;
GBitmap *oops_bitmap;
tm tm_time;
