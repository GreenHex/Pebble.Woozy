//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "clock.h"

#if defined( PBL_COLOR )
#define NUM_PBL_64_COLOURS 64

const uint32_t PBL_64_COLOURS[ NUM_PBL_64_COLOURS ] = {
  0x000000, 0xFFFFFF, 0xAAAAAA, 0x555555, 0xFFFFAA, 0xFFFF55, 0xFFAA55, 0xFF5500,
  0xFF0000, 0xFF0055, 0xFF5555, 0xFFAAAA, 0xFFFF00, 0xFFAA00, 0xAA5500, 0xAA5555,
  0xAA0000, 0xFF00AA, 0xFF55AA, 0xFFAAFF, 0x550000, 0xAA0055, 0xFF00FF, 0xFF55FF,
  0x550055, 0xAA00AA, 0xAA55AA, 0x000055, 0x5500AA, 0xAA00FF, 0xAA55FF, 0x0000AA,
  0x5500FF, 0x5555AA, 0x0055AA, 0x55AAAA, 0x55AA55, 0x00AA00, 0x00FF00, 0x55FF00,
  0xAAFF55, 0xAAFF00, 0x55AA00, 0x005500, 0x005555, 0xAAAA55, 0x555500, 0xAAAA00,
  0xAAFFAA, 0x55FF55, 0x00FF55, 0x00AA55, 0x00AAAA, 0x00AAFF, 0x0000FF, 0x5555FF,
  0xAAAAFF, 0x55FFAA, 0x00FFAA, 0x00FFFF, 0x55AAFF, 0x0055FF, 0x55FFFF, 0xAAFFFF
};
#endif /* PBL_COLOR */

#define NUM_DIGITS 12

static Layer *window_layer = 0;
static BitmapLayer *clockface_layer = 0;
static GBitmap *clockface_bitmap = 0;
static Layer *digit_layer[ NUM_DIGITS ] = { 0 };
static Layer *hour_layer = 0;
static Layer *min_layer = 0;
static char digit_str[3] = "0";
static PropertyAnimation *digit_prop_animation = 0;
static PropertyAnimation *hour_hand_prop_animation = 0;
static PropertyAnimation *min_hand_prop_animation = 0;
extern tm tm_time;

bool is_X_in_range( int a, int b, int x ) { return ( ( b > a ) ? ( ( x >= a ) && ( x < b ) ) : ( ( x >= a ) || ( x < b ) ) ); };

static void print_rect( char *str, GRect rect ) {
  APP_LOG( APP_LOG_LEVEL_INFO, "%s: ( %d, %d, %d, %d )", str, rect.origin.x, rect.origin.y, rect.size.w, rect.size.h );
}

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

static void grect_setter( void *subject, GRect rect ) {
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "grect_setter()" );
  #endif
  HAND_LAYER_DATA *hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( (Layer *) subject );
  hand_layer_data->current_rect = rect;
  // print_rect( "grect_setter()", rect );
}

static GRect grect_getter( void *subject ) {
  GRect rect = GRect( 0, 0, 0, 0 );
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "grect_getter()" );
  #endif
  HAND_LAYER_DATA *hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( (Layer *) subject );
  rect = hand_layer_data->current_rect;
  // print_rect( "grect_getter()", rect );
  return rect;
}

static const PropertyAnimationImplementation hand_animation_implementation = {
  .base = {
    .update = (AnimationUpdateImplementation) property_animation_update_grect,
  },
    .accessors = {
    .setter = { .grect = (const GRectSetter) grect_setter },
    .getter = { .grect = (const GRectGetter) grect_getter },
  },
};

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global  
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%02d:%02d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
  #endif
  int mod_val = 10;
  uint32_t hour_angle = ( TRIG_MAX_ANGLE * ( ( ( tm_time.tm_hour % 12 ) * 6 ) + ( tm_time.tm_min / 10 ) ) ) / ( 12 * 6 );
  uint32_t min_angle = TRIG_MAX_ANGLE * tm_time.tm_min / 60;
  HAND_LAYER_DATA *hour_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( hour_layer );
  HAND_LAYER_DATA *min_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( min_layer );

  hour_hand_layer_data->home_rect.size.w = ( sin_lookup( hour_angle ) * HOUR_HAND_LENGTH / TRIG_MAX_RATIO ) + PBL_DISPLAY_WIDTH / 2;
  hour_hand_layer_data->home_rect.size.h = ( -cos_lookup( hour_angle ) * HOUR_HAND_LENGTH / TRIG_MAX_RATIO ) + PBL_DISPLAY_HEIGHT / 2;
  min_hand_layer_data->home_rect.size.w = ( sin_lookup( min_angle ) * MIN_HAND_LENGTH / TRIG_MAX_RATIO ) + PBL_DISPLAY_WIDTH / 2;
  min_hand_layer_data->home_rect.size.h = ( -cos_lookup( min_angle ) * MIN_HAND_LENGTH / TRIG_MAX_RATIO ) + PBL_DISPLAY_HEIGHT / 2;
  // print_rect( "time_update_hour", hour_hand_layer_data->home_rect );
  // print_rect( "time_update_min", min_hand_layer_data->home_rect );
  
  if ( tm_time.tm_sec % 20 ) {
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
  } else {
    tick_timer_service_unsubscribe();
    
    Animation **digit_animation_array = (Animation**) malloc( ( NUM_DIGITS + 2 ) * sizeof( Animation* ) );
    Animation *digit_animation = 0;
    for ( int i = 0; i < NUM_DIGITS; i++ ) {
      digit_prop_animation = property_animation_create_layer_frame( digit_layer[i], 
          &( ( ( DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_frame ), 
          &( ( ( DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->home_frame ) );
      
      digit_animation = property_animation_get_animation( digit_prop_animation );
  
      animation_set_curve( digit_animation, AnimationCurveEaseOut );
      animation_set_delay( digit_animation, ANIMATION_DELAY );
      animation_set_duration( digit_animation, ANIMATION_DURATION );
      digit_animation_array[i] = digit_animation;
    }
    
    hour_hand_prop_animation = property_animation_create( &hand_animation_implementation, NULL, NULL, NULL );
    property_animation_subject( hour_hand_prop_animation, (void *) &hour_layer, true );
    property_animation_from( hour_hand_prop_animation, &( hour_hand_layer_data->current_rect ), sizeof( hour_hand_layer_data->current_rect ), true );
    property_animation_to( hour_hand_prop_animation, &( hour_hand_layer_data->home_rect ), sizeof( hour_hand_layer_data->home_rect ), true );
    Animation *hour_hand_animation = property_animation_get_animation( hour_hand_prop_animation );
    animation_set_curve( hour_hand_animation, AnimationCurveEaseOut );
    animation_set_delay( hour_hand_animation, ANIMATION_DELAY );
    animation_set_duration( hour_hand_animation, ANIMATION_DURATION );
    
    min_hand_prop_animation = property_animation_create( &hand_animation_implementation, NULL, NULL, NULL );
    property_animation_subject( min_hand_prop_animation, (void *) &min_layer, true );
    property_animation_from( min_hand_prop_animation, &( min_hand_layer_data->current_rect ), sizeof( min_hand_layer_data->current_rect ), true );
    property_animation_to( min_hand_prop_animation, &( min_hand_layer_data->home_rect ), sizeof( min_hand_layer_data->home_rect ), true );
    Animation *min_hand_animation = property_animation_get_animation( min_hand_prop_animation );
    animation_set_curve( min_hand_animation, AnimationCurveEaseOut );
    animation_set_delay( min_hand_animation, ANIMATION_DELAY );
    animation_set_duration( min_hand_animation, ANIMATION_DURATION );
    
    digit_animation_array[ NUM_DIGITS ] = hour_hand_animation;
    digit_animation_array[ NUM_DIGITS + 1 ] = min_hand_animation;
    Animation *spawn = animation_spawn_create_from_array( digit_animation_array, NUM_DIGITS + 4 );
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
  
  graphics_context_set_text_color( ctx, GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ) );
  snprintf( digit_str, sizeof( digit_str), "%u",  ( ( DIGIT_LAYER_DATA *) layer_get_data( layer ) )->digit );
  layer_bounds.origin.y -= DIGIT_TXT_VERT_ADJ;
  graphics_draw_text( ctx, digit_str, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ), layer_bounds,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL );
}

static void hand_layer_update_proc( Layer *layer, GContext *ctx ) {
  HAND_LAYER_DATA *hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( layer );
  GPoint start_pt = GPoint( hand_layer_data->current_rect.origin.x, hand_layer_data->current_rect.origin.y );
  GPoint end_pt = GPoint( hand_layer_data->current_rect.size.w, hand_layer_data->current_rect.size.h );
  
  graphics_context_set_stroke_color( ctx, BG_BITMAP_BG_COLOUR );
  graphics_context_set_stroke_width( ctx, hand_layer_data->stroke_width + 2 );
  graphics_draw_line( ctx, start_pt, end_pt );
  
  graphics_context_set_stroke_color( ctx, GColorFromHEX( hand_layer_data->colour ) );
  graphics_context_set_stroke_width( ctx, hand_layer_data->stroke_width );
  graphics_draw_line( ctx, start_pt, end_pt ); 
  
  graphics_context_set_fill_color( ctx, GColorBlack );
  graphics_fill_circle( ctx, start_pt, hand_layer_data->hole_radius );
}

void clock_init( Window *window ) {
  window_layer = window_get_root_layer( window );
  GRect window_bounds = layer_get_bounds( window_layer );
  
  clockface_layer = bitmap_layer_create( window_bounds );
  layer_add_child( window_layer, bitmap_layer_get_layer( clockface_layer ) );
  #ifdef SHOW_BACKGROUND_BITMAP_IMAGE
  clockface_bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_CLOCKFACE );
  bitmap_layer_set_bitmap( clockface_layer, clockface_bitmap );
  #else
  window_set_background_color( window, GColorBlack );
  #endif
  
  GRect digit_layer_frame_rect;
  DIGIT_LAYER_DATA *digit_layer_data = 0;
  for ( int i = 0; i < NUM_DIGITS; i ++ ) {
    digit_layer_frame_rect = GRect( DIGIT_LOCATIONS.points[i].x, DIGIT_LOCATIONS.points[i].y,
                                   DIGIT_RECT_SIZE_W, DIGIT_RECT_SIZE_H ); 
    digit_layer[i] = layer_create_with_data( digit_layer_frame_rect, sizeof( DIGIT_LAYER_DATA ) );
    digit_layer_data = ( DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] );
    digit_layer_data->digit = i + 1;
    digit_layer_data->colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF );
    digit_layer_data->home_frame = digit_layer_frame_rect;
    digit_layer_data->current_frame = digit_layer_frame_rect;
    layer_set_update_proc( digit_layer[i], digit_layer_update_proc );
    layer_add_child( bitmap_layer_get_layer( clockface_layer ), digit_layer[i] );
  }
  
  GPoint home_start_point;
  GPoint home_end_point;
  GPoint current_start_point;
  GPoint current_end_point;
  HAND_LAYER_DATA *hand_layer_data = 0;
  
  hour_layer = layer_create_with_data( window_bounds, sizeof( HAND_LAYER_DATA ) );
  hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( hour_layer ); 
  hand_layer_data->colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF );
  hand_layer_data->stroke_width = HOUR_HAND_THK;
  hand_layer_data->hole_radius = 2;
  hand_layer_data->layer_frame = window_bounds;
  hand_layer_data->home_rect = HOUR_RECT;
  hand_layer_data->current_rect = HOUR_RECT;
  layer_set_update_proc( hour_layer, hand_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), hour_layer );

  min_layer = layer_create_with_data( window_bounds, sizeof( HAND_LAYER_DATA ) );
  hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( min_layer );
  hand_layer_data->colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF );
  hand_layer_data->stroke_width = MIN_HAND_THK;
  hand_layer_data->hole_radius = 1;
  hand_layer_data->layer_frame = window_bounds;
  hand_layer_data->home_rect = MIN_RECT;
  hand_layer_data->current_rect = MIN_RECT;
  layer_set_update_proc( min_layer, hand_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), min_layer );

  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
  
  draw_clock();
}

void clock_deinit( void ) {
  tick_timer_service_unsubscribe();
  if ( min_layer ) layer_destroy( min_layer );
  if ( hour_layer ) layer_destroy( hour_layer );
  for ( int i = 0; i < NUM_DIGITS; i ++ ) {
    if ( digit_layer[i] ) layer_destroy( digit_layer[i] );
  }
  if ( clockface_layer ) bitmap_layer_destroy( clockface_layer );
  if ( clockface_bitmap ) gbitmap_destroy( clockface_bitmap );
}
