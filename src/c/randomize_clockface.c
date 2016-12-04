//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "randomize_clockface.h"

extern BitmapLayer *clockface_layer;
extern Layer *digit_layer[];
extern Layer *hour_layer;
extern Layer *min_layer;

static int16_t get_next_random_value( int16_t x, int16_t min_val, int16_t max_val, int mod_val ) {
  int rand_val = rand() % mod_val;
  int is_plus_not_minus = rand() % 2;
  int16_t tmp_val = is_plus_not_minus ? x + rand_val : x - rand_val;
  if ( ( tmp_val < min_val) || ( tmp_val > max_val ) ) {
    tmp_val = is_plus_not_minus ? x - rand_val : x + rand_val;
  }
  return tmp_val;
}

void randomize_clockface( void ) {
  GRect layer_uo_bounds = layer_get_unobstructed_bounds( bitmap_layer_get_layer( clockface_layer ) );
  int16_t display_width = layer_uo_bounds.size.w;
  int16_t display_height = layer_uo_bounds.size.h;
  
  srand ( time( NULL ) );
  
  int mod_val = 10;
  
  for ( int i = 0; i < NUM_DIGITS; i++ ) {
    GRect current_rect = ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_rect; // layer_get_frame( digit_layer[i] );
    current_rect.origin = (GPoint) {
      .x = get_next_random_value( current_rect.origin.x, 0, display_width - current_rect.size.w, mod_val ),
      .y = get_next_random_value( current_rect.origin.y, 0, display_height - current_rect.size.h, mod_val )
    };
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_rect = current_rect;
    layer_set_frame( digit_layer[i], current_rect );
  }
  
  HAND_LAYER_DATA *hour_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( hour_layer );
  hour_hand_layer_data->current_rect.origin = (GPoint) {
    .x = get_next_random_value( hour_hand_layer_data->current_rect.origin.x, 
                               0, display_width, mod_val / 2 ),
    .y = get_next_random_value( hour_hand_layer_data->current_rect.origin.y, 
                               0, display_height, mod_val / 2 )
  };
  hour_hand_layer_data->current_rect.size = (GSize) {
    .w = get_next_random_value( hour_hand_layer_data->current_rect.size.w, 
                               0, display_width, mod_val / 2 ),
    .h = get_next_random_value( hour_hand_layer_data->current_rect.size.h, 
                               0, display_height, mod_val / 2 )
  };
  HAND_LAYER_DATA *min_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( min_layer );
  min_hand_layer_data->current_rect.origin = (GPoint) {
    .x = get_next_random_value( min_hand_layer_data->current_rect.origin.x, 
                               0, display_width, mod_val / 2 ),
    .y = get_next_random_value( min_hand_layer_data->current_rect.origin.y, 
                               0, display_height, mod_val / 2 )
  };
  min_hand_layer_data->current_rect.size = (GSize) {
    .w = get_next_random_value( min_hand_layer_data->current_rect.size.w, 
                               0, display_width, mod_val / 2 ),
    .h = get_next_random_value( min_hand_layer_data->current_rect.size.h, 
                               0, display_height, mod_val / 2 )
  };
}
