//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "clock.h"

#define NUM_DIGITS 12

static Layer *window_layer = 0;
static BitmapLayer *clockface_layer = 0;
static GBitmap *clockface_bitmap = 0;
static Layer *digit_layer[ NUM_DIGITS ] = { 0 };
static char digit_str[3] = "0";
static PropertyAnimation *digit_prop_animation = 0;
extern tm tm_time;

bool is_X_in_range( int a, int b, int x ) { return ( ( b > a ) ? ( ( x >= a ) && ( x < b ) ) : ( ( x >= a ) || ( x < b ) ) ); };

int16_t get_next_random_value( int16_t x, int16_t min_val, int16_t max_val, int mod_val ) {
  int rand_val = rand() % mod_val;
  int is_plus_not_minus = rand() % 2;
  int16_t tmp_val = is_plus_not_minus ? x + rand_val : x - rand_val;
  if ( ( tmp_val < min_val) || ( tmp_val > max_val ) ) {
    tmp_val = is_plus_not_minus ? x - rand_val : x + rand_val;
  }
  return tmp_val;
}

static void draw_clock( void ) {
  time_t now = time( NULL );
  tm_time = *localtime( &now ); // copy to global
  layer_mark_dirty( window_layer );
}

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global  
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%02d:%02d", 
          tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
  #endif
  
  if ( tm_time.tm_sec % 60 ) {
    for ( int i = 0; i < NUM_DIGITS; i++ ) {
      GRect current_frame = layer_get_frame( digit_layer[i] );
      current_frame.origin.x = get_next_random_value( current_frame.origin.x, 0, PBL_DISPLAY_WIDTH - current_frame.size.w, 20 );
      current_frame.origin.y = get_next_random_value( current_frame.origin.y, 0, PBL_DISPLAY_HEIGHT - current_frame.size.h, 20 );
      ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_frame = current_frame;
      layer_set_frame( digit_layer[i], current_frame );
    }
  } else {
    tick_timer_service_unsubscribe();
    Animation **digit_animation_array = (Animation**) malloc( NUM_DIGITS * sizeof( Animation* ) );
    for ( int i = 0; i < NUM_DIGITS; i++ ) {
      digit_prop_animation = property_animation_create_layer_frame( digit_layer[i], 
          &( ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_frame ), 
          &( ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->home_frame ) );
      
      Animation *digit_animation = property_animation_get_animation( digit_prop_animation );
  
      animation_set_curve( digit_animation, AnimationCurveEaseOut );
      animation_set_delay( digit_animation, 5 );
      animation_set_duration( digit_animation, 2000 );
      digit_animation_array[i] = digit_animation;
    }
    Animation *spawn = animation_spawn_create_from_array( digit_animation_array, NUM_DIGITS );
    animation_set_play_count( spawn, 1 );
    animation_schedule( spawn );
    free( digit_animation_array );
    tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
  }
}

static void digit_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  // graphics_context_set_fill_color( ctx, GColorLightGray );
  // graphics_fill_rect( ctx, layer_bounds, 0, GCornerNone );
  
  graphics_context_set_text_color( ctx, GColorWhite );
  snprintf( digit_str, sizeof( digit_str), "%u",  ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->digit );
  layer_bounds.origin.y -= DIGIT_TXT_VERT_ADJ;
  graphics_draw_text( ctx, digit_str, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ), layer_bounds,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL );
}

/*
static void create_animation( void ) {
  digit_prop_animation = property_animation_create_layer_frame( digit_layer, 
        &( ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer ) )->current_frame ), 
        &( ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer ) )->home_frame ) );
}
*/

void clock_init( Window *window ) {
  window_layer = window_get_root_layer( window );
  GRect window_bounds = layer_get_bounds( window_layer );
  
  clockface_bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_CLOCKFACE );
  clockface_layer = bitmap_layer_create( window_bounds );
  bitmap_layer_set_bitmap( clockface_layer, clockface_bitmap );
  layer_add_child( window_layer, bitmap_layer_get_layer( clockface_layer ) );
  GRect digit_layer_frame_rect;
  
  for ( int i = 0; i < NUM_DIGITS; i ++ ) {
    digit_layer_frame_rect = GRect( DIGIT_LOCATIONS.points[i].x, DIGIT_LOCATIONS.points[i].y,
                                   DIGIT_RECT_SIZE_W, DIGIT_RECT_SIZE_H ); 
    digit_layer[i] = layer_create_with_data( digit_layer_frame_rect, sizeof( DIGIT_LAYER_DATA ) );
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->digit = i + 1;
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->home_frame = digit_layer_frame_rect;
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_frame = digit_layer_frame_rect;
    layer_set_update_proc( digit_layer[i], digit_layer_update_proc );
    layer_add_child( bitmap_layer_get_layer( clockface_layer ), digit_layer[i] );
  }
  
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
  
  draw_clock();
}

void clock_deinit( void ) {
  tick_timer_service_unsubscribe();
  for ( int i = 0; i < NUM_DIGITS; i ++ ) {
    if ( digit_layer[i] ) layer_destroy( digit_layer[i] );
  }
  if ( clockface_layer ) bitmap_layer_destroy( clockface_layer );
  if ( clockface_bitmap ) gbitmap_destroy( clockface_bitmap );
}
