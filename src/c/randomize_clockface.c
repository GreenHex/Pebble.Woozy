//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "randomize_clockface.h"

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
  int mod_val = 10;
  HAND_LAYER_DATA *hour_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( hour_layer );
  HAND_LAYER_DATA *min_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( min_layer );

  for ( int i = 0; i < NUM_DIGITS; i++ ) {
    GRect current_frame = layer_get_frame( digit_layer[i] );
    current_frame.origin.x = get_next_random_value( current_frame.origin.x, 0, PBL_DISPLAY_WIDTH - current_frame.size.w, mod_val );
    current_frame.origin.y = get_next_random_value( current_frame.origin.y, 0, PBL_DISPLAY_HEIGHT - current_frame.size.h, mod_val );
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_frame = current_frame;
    layer_set_frame( digit_layer[i], current_frame );
  }
  hour_hand_layer_data->current_rect.origin.x = get_next_random_value( hour_hand_layer_data->current_rect.origin.x, 
                                                                      0, PBL_DISPLAY_WIDTH, mod_val / 2 );
  hour_hand_layer_data->current_rect.origin.y = get_next_random_value( hour_hand_layer_data->current_rect.origin.y, 
                                                                      0, PBL_DISPLAY_HEIGHT, mod_val / 2 );
  hour_hand_layer_data->current_rect.size.w = get_next_random_value( hour_hand_layer_data->current_rect.size.w, 
                                                                    0, PBL_DISPLAY_WIDTH, mod_val / 2 );
  hour_hand_layer_data->current_rect.size.h = get_next_random_value( hour_hand_layer_data->current_rect.size.h, 
                                                                    0, PBL_DISPLAY_HEIGHT, mod_val / 2 );
  min_hand_layer_data->current_rect.origin.x = get_next_random_value( min_hand_layer_data->current_rect.origin.x, 
                                                                     0, PBL_DISPLAY_WIDTH, mod_val / 2 );
  min_hand_layer_data->current_rect.origin.y = get_next_random_value( min_hand_layer_data->current_rect.origin.y, 
                                                                     0, PBL_DISPLAY_HEIGHT, mod_val / 2 );
  min_hand_layer_data->current_rect.size.w = get_next_random_value( min_hand_layer_data->current_rect.size.w, 
                                                                   0, PBL_DISPLAY_WIDTH, mod_val / 2 );
  min_hand_layer_data->current_rect.size.h = get_next_random_value( min_hand_layer_data->current_rect.size.h, 
                                                                   0, PBL_DISPLAY_HEIGHT, mod_val / 2 );
}
