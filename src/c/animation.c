//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#include <pebble.h>
#include "animation.h"

// extern Layer *digit_layer[];
extern Layer *hour_layer;
extern Layer *min_layer;

static PropertyAnimation *digit_prop_animation = 0;
static PropertyAnimation *hour_hand_prop_animation = 0;
static PropertyAnimation *min_hand_prop_animation = 0;

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

void start_animation( void ) {
  HAND_LAYER_DATA *hour_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( hour_layer );
  HAND_LAYER_DATA *min_hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( min_layer );

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
}