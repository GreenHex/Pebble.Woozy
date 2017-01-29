//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "global.h"
#include "animation.h"

// #define DEBUG

extern BitmapLayer *clockface_layer;
extern Layer *snooze_layer;
extern Layer *digit_layer[];
extern Layer *hour_layer;
extern Layer *min_layer;
extern Layer *day_layer;
extern Layer *date_layer;
extern BitmapLayer *oops_layer;

#define NUM_ANIMATIONS ( NUM_DIGITS + 5 )

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

void start_second_animation( void *data ) { 
  for ( int i = 0; i < NUM_DIGITS; i ++ ) { 
    ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->home_rect = GRect( DIGIT_LOCATIONS.points[i].x, DIGIT_LOCATIONS.points[i].y,
                                                                                 DIGIT_RECT_SIZE_W, DIGIT_RECT_SIZE_H );;
  }
  if ( launch_reason() == APP_LAUNCH_USER ) {
    start_animation( 200, 1000, AnimationCurveEaseInOut, false );
  } else {
    start_animation( 10, 800, AnimationCurveEaseIn, false );
  }
}

static void random_wait_timer( void *data ) {
  vibes_enqueue_custom_pattern( single_vibe_pattern );
  layer_set_hidden( bitmap_layer_get_layer( oops_layer ), false );
  app_timer_register( 500, start_second_animation, 0 );
}

static void digit_grect_setter( void *subject, GRect rect ) {
  ( (DIGIT_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_rect = rect;
  log_rect( "setter:", rect );
  layer_set_frame( (Layer *) subject, ( (DIGIT_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_rect );
  layer_mark_dirty( (Layer *) subject );
}

static GRect digit_grect_getter( void *subject ) {
  return ( ( DIGIT_LAYER_DATA *) layer_get_data( (Layer *) subject ) )->current_rect;
}

static void digit_animation_implementation_teardown( Animation *animation ) {
  if ( second_animation ) {
    second_animation = false;
    app_timer_register( ( rand() % 4 + 2 ) * 1000, random_wait_timer, 0 );
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

static Animation *animate_layer_origin( Layer *layer, const PropertyAnimationImplementation *prop_animation_implementation, 
                                int delay_ms, int duration_ms, AnimationCurve anim_curve ) {
  PropertyAnimation *layer_prop_animation = property_animation_create( prop_animation_implementation, NULL, NULL, NULL );
  property_animation_subject( layer_prop_animation, (void *) &layer, true );
  DIGIT_LAYER_DATA *layer_data = (DIGIT_LAYER_DATA *) layer_get_data( layer );
  property_animation_from( layer_prop_animation, (void *) &( layer_data->current_rect ),
                          sizeof( layer_data->current_rect ), true );
  property_animation_to( layer_prop_animation, (void *) &( layer_data->home_rect ),
                        sizeof( layer_data->home_rect ), true );
  Animation *digit_animation = property_animation_get_animation( layer_prop_animation );
  animation_set_curve( digit_animation, anim_curve );
  animation_set_delay( digit_animation, delay_ms );
  animation_set_duration( digit_animation, duration_ms );
  return ( digit_animation );
}

static Animation *animate_layer_rect( Layer *layer, const PropertyAnimationImplementation *hand_animation_implementation, 
                               int delay_ms, int duration_ms, AnimationCurve anim_curve ) {
  PropertyAnimation *hand_prop_animation = property_animation_create( hand_animation_implementation, NULL, NULL, NULL );
  property_animation_subject( hand_prop_animation, (void *) &layer, true );
  HAND_LAYER_DATA *hand_layer_data = (HAND_LAYER_DATA *) layer_get_data( layer );
  hand_prop_animation = property_animation_create( hand_animation_implementation,
                                                  (void *) &( hand_layer_data->current_rect ), NULL, NULL );
  property_animation_from( hand_prop_animation, &( hand_layer_data->current_rect ),
                          sizeof( hand_layer_data->current_rect ), true );
  property_animation_to( hand_prop_animation, &( hand_layer_data->home_rect ),
                        sizeof( hand_layer_data->home_rect ), true );
  Animation *hand_animation = property_animation_get_animation( hand_prop_animation );
  animation_set_curve( hand_animation, anim_curve );
  animation_set_delay( hand_animation, delay_ms );
  animation_set_duration( hand_animation, duration_ms );
  return ( hand_animation );
}

void start_animation( int delay_ms, int duration_ms, AnimationCurve anim_curve, bool do_second_animation ) {
  tick_timer_service_unsubscribe();
  second_animation = do_second_animation; // global
  
  Animation **digit_animation_array = (Animation **) malloc( ( NUM_ANIMATIONS ) * sizeof( Animation* ) );

  for ( int i = 0; i < NUM_DIGITS; i++ ) {
    digit_animation_array[ i ] = animate_layer_origin( digit_layer[ i ], &digit_animation_implementation, delay_ms, duration_ms, anim_curve );;
  }
  digit_animation_array[ NUM_DIGITS ] = animate_layer_origin( snooze_layer, &digit_animation_implementation, delay_ms, duration_ms, anim_curve );
  digit_animation_array[ NUM_DIGITS + 1 ] = animate_layer_origin( date_layer, &digit_animation_implementation, delay_ms, duration_ms, anim_curve );
  digit_animation_array[ NUM_DIGITS + 2 ] = animate_layer_origin( day_layer, &digit_animation_implementation, delay_ms, duration_ms, anim_curve );
  digit_animation_array[ NUM_DIGITS + 3 ] = animate_layer_rect( hour_layer, &hand_animation_implementation, delay_ms, duration_ms, anim_curve );
  digit_animation_array[ NUM_DIGITS + 4 ] = animate_layer_rect( min_layer, &hand_animation_implementation, delay_ms, duration_ms, anim_curve );
  
  Animation *spawn = 0;
  spawn = animation_spawn_create_from_array( digit_animation_array, NUM_ANIMATIONS );
  animation_set_play_count( spawn, 1 );
  animation_schedule( spawn );
  free( digit_animation_array );
  layer_set_hidden( bitmap_layer_get_layer( oops_layer ), true );
}
