//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "randomize_clockface.h"

extern BitmapLayer *clockface_layer;
extern Layer *snooze_layer;
extern Layer *digit_layer[];
extern Layer *hour_layer;
extern Layer *min_layer;
extern Layer *day_layer;
extern Layer *date_layer;

static int16_t get_next_random_value( int16_t x, int16_t min_val, int16_t max_val, int mod_val ) {
  int rand_val = rand() % mod_val;
  int is_plus_not_minus = rand() % 2;
  int16_t tmp_val = is_plus_not_minus ? x + rand_val : x - rand_val;
  if ( ( tmp_val < min_val) || ( tmp_val > max_val ) ) {
    tmp_val = is_plus_not_minus ? x - rand_val : x + rand_val;
  }
  return tmp_val;
}

static void layer_set_random_origin( Layer *layer, int16_t display_width, int16_t display_height, int mod_val ) {
  GRect *current_rect = &( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->current_rect );
  current_rect->origin = (GPoint) {
    .x = get_next_random_value( current_rect->origin.x, 0, display_width - current_rect->size.w, mod_val ),
    .y = get_next_random_value( current_rect->origin.y, 0, display_height - current_rect->size.h, mod_val )
  };
  layer_set_frame( layer, *current_rect );
}

static void layer_set_random_rect( Layer *layer, int16_t display_width, int16_t display_height, int mod_val ) {
  HAND_LAYER_DATA *hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( layer );
  hand_layer_data->current_rect = (GRect) {
    .origin = (GPoint) {
      .x = get_next_random_value( hand_layer_data->current_rect.origin.x, 
                                 0, display_width, mod_val / 2 ),
      .y = get_next_random_value( hand_layer_data->current_rect.origin.y, 
                                 0, display_height, mod_val / 2 )
    },
    .size  = (GSize) {
      .w = get_next_random_value( hand_layer_data->current_rect.size.w, 
                                 0, display_width, mod_val / 2 ),
      .h = get_next_random_value( hand_layer_data->current_rect.size.h, 
                                 0, display_height, mod_val / 2 )
    }
  };
}

void randomize_clockface( void ) {
  GRect layer_uo_bounds = layer_get_unobstructed_bounds( bitmap_layer_get_layer( clockface_layer ) );
  int16_t display_width = layer_uo_bounds.size.w;
  int16_t display_height = layer_uo_bounds.size.h; 
  srand ( time( NULL ) );
  int mod_val = 10;
  
  layer_set_random_origin( snooze_layer, display_width, display_height, mod_val );
  
  for ( int i = 0; i < NUM_DIGITS; i++ ) {
    layer_set_random_origin( digit_layer[i], display_width, display_height, mod_val );
  }
  layer_set_random_origin( day_layer, display_width, display_height, mod_val );
  layer_set_random_origin( date_layer, display_width, display_height, mod_val );
  
  layer_set_random_rect( hour_layer, display_width, display_height, mod_val );
  layer_set_random_rect( min_layer, display_width, display_height, mod_val );
}
