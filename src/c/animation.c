//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "animation.h"

// #define DEBUG

extern BitmapLayer *clockface_layer;
extern Layer *digit_layer[];
extern Layer *hour_layer;
extern Layer *min_layer;
extern BitmapLayer *oops_layer;

#define NUM_ANIMATIONS ( NUM_DIGITS + 2 )

static bool second_animation = true;

static uint32_t const one_segment[] = { 200, 200 };
VibePattern single_vibe_pattern = {
  .durations = one_segment,
  .num_segments = ARRAY_LENGTH( one_segment ),
};

static void log_rect( char *str, GRect rect ) {
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "%s: ( %d, %d, %d, %d )", str, rect.origin.x, rect.origin.y, rect.size.w, rect.size.h );
  #endif
}

static void start_second_animation( void *data ) { 
  for ( int i = 0; i < NUM_DIGITS; i ++ ) { 
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->home_rect = GRect( DIGIT_LOCATIONS.points[i].x, DIGIT_LOCATIONS.points[i].y,
                                                                                 DIGIT_RECT_SIZE_W, DIGIT_RECT_SIZE_H );;
  }
  start_animation( 200, 1000, false );
}

static void digit_grect_setter( void *subject, GRect rect ) {
  ( (DIGIT_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_rect = rect;
  log_rect( "setter:", rect );
  layer_set_frame( (Layer *) subject, ( ( DIGIT_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_rect );
  layer_mark_dirty( (Layer *) subject );
}

static GRect digit_grect_getter( void *subject ) {
  return ( ( DIGIT_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_rect;
}

static void digit_animation_implementation_teardown( Animation *animation ) {
  if ( second_animation ) {
    second_animation = false;
    vibes_enqueue_custom_pattern( single_vibe_pattern );
    layer_set_hidden( bitmap_layer_get_layer( oops_layer ), false );
    app_timer_register( 500, start_second_animation, 0 );
  }
}

static const PropertyAnimationImplementation digit_animation_implementation = {
  .base = {
    .update = (AnimationUpdateImplementation) property_animation_update_grect,
    .teardown = digit_animation_implementation_teardown,
  },
    .accessors = {
      .setter = { .grect = (const GRectSetter) digit_grect_setter },
      .getter = { .grect = (const GRectGetter) digit_grect_getter },
  },
};

static void hand_grect_setter( void *subject, GRect rect ) {
  *((GRect *) subject) = rect;
}

static GRect hand_grect_getter( void *subject ) {
  return *((GRect *) subject);
}

static const PropertyAnimationImplementation hand_animation_implementation = {
  .base = {
    .update = (AnimationUpdateImplementation) property_animation_update_grect,
  },
    .accessors = {
      .setter = { .grect = (const GRectSetter) hand_grect_setter },
      .getter = { .grect = (const GRectGetter) hand_grect_getter },
  },
};

void start_animation( int delay_ms, int duration_ms, bool do_second_animation ) {
  tick_timer_service_unsubscribe();
  second_animation = do_second_animation;
  
  Animation **digit_animation_array = 0;
  digit_animation_array = (Animation**) malloc( ( NUM_ANIMATIONS ) * sizeof( Animation* ) );

  static PropertyAnimation *digit_prop_animation[NUM_DIGITS] = { 0 };
  static PropertyAnimation *hour_hand_prop_animation = 0;
  static PropertyAnimation *min_hand_prop_animation = 0;
  
  Animation *digit_animation = 0;
  DIGIT_LAYER_DATA *digit_layer_data = 0;
  for ( int i = 0; i < NUM_DIGITS; i++ ) {
    digit_layer_data = (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] );
    digit_prop_animation[i] = property_animation_create( &digit_animation_implementation, NULL, NULL, NULL );
    property_animation_subject( digit_prop_animation[i], (void *) &(digit_layer[i]), true );
    property_animation_from( digit_prop_animation[i], (void *) &( digit_layer_data->current_rect ),
                            sizeof( digit_layer_data->current_rect ), true );
    property_animation_to( digit_prop_animation[i], (void *) &( digit_layer_data->home_rect ),
                          sizeof( digit_layer_data->home_rect ), true ); 
    
    digit_animation = property_animation_get_animation( digit_prop_animation[i] );
    animation_set_curve( digit_animation, AnimationCurveEaseOut );
    animation_set_delay( digit_animation, delay_ms );
    animation_set_duration( digit_animation, duration_ms );
    digit_animation_array[i] = digit_animation;
  }
  
  HAND_LAYER_DATA *hour_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( hour_layer );
  hour_hand_prop_animation = property_animation_create( &hand_animation_implementation,
                                                       (void *) &( hour_hand_layer_data->current_rect ), NULL, NULL );
  property_animation_from( hour_hand_prop_animation, &( hour_hand_layer_data->current_rect ),
                          sizeof( hour_hand_layer_data->current_rect ), true );
  property_animation_to( hour_hand_prop_animation, &( hour_hand_layer_data->home_rect ),
                        sizeof( hour_hand_layer_data->home_rect ), true );
  Animation *hour_hand_animation = property_animation_get_animation( hour_hand_prop_animation );
  animation_set_curve( hour_hand_animation, AnimationCurveEaseOut );
  animation_set_delay( hour_hand_animation, delay_ms );
  animation_set_duration( hour_hand_animation, duration_ms );
  digit_animation_array[ NUM_DIGITS ] = hour_hand_animation;

  HAND_LAYER_DATA *min_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( min_layer );
  min_hand_prop_animation = property_animation_create( &hand_animation_implementation,
                                                      (void *) &( min_hand_layer_data->current_rect ), NULL, NULL );
  property_animation_from( min_hand_prop_animation, &( min_hand_layer_data->current_rect ),
                          sizeof( min_hand_layer_data->current_rect ), true );
  property_animation_to( min_hand_prop_animation, &( min_hand_layer_data->home_rect ),
                        sizeof( min_hand_layer_data->home_rect ), true );
  Animation *min_hand_animation = property_animation_get_animation( min_hand_prop_animation );
  animation_set_curve( min_hand_animation, AnimationCurveEaseOut );
  animation_set_delay( min_hand_animation, delay_ms );
  animation_set_duration( min_hand_animation, duration_ms );
  digit_animation_array[ NUM_DIGITS + 1 ] = min_hand_animation;
  
  Animation *spawn = 0;
  spawn = animation_spawn_create_from_array( digit_animation_array, NUM_ANIMATIONS );
  animation_set_play_count( spawn, 1 );
  animation_schedule( spawn );
  free( digit_animation_array );
  layer_set_hidden( bitmap_layer_get_layer( oops_layer ), true );
}
