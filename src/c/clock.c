#include <pebble.h>
#include "global.h"
#include "clock.h"

static Layer *window_layer = 0;
static BitmapLayer *clockface_layer = 0;
static GBitmap *clockface_bitmap = 0;
extern tm tm_time;

bool is_X_in_range( int a, int b, int x ) { return ( ( b > a ) ? ( ( x >= a ) && ( x < b ) ) : ( ( x >= a ) || ( x < b ) ) ); };

void draw_clock( void ) {
  time_t now = time( NULL );
  tm_time = *localtime( &now ); // copy to global
  layer_mark_dirty( window_layer );
}

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global  
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%d:%d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
  #endif
  layer_mark_dirty( window_layer );  
}

/*
static void hour_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_draw_bitmap_in_rect( ctx, bitmap_webos_clockface, layer_bounds );
  graphics_context_set_compositing_mode( ctx, GCompOpSet );
  uint32_t hour_angle = ( TRIG_MAX_ANGLE * ( ( ( tm_time.tm_hour % 12 ) * 6 ) + ( tm_time.tm_min / 10 ) ) ) / ( 12 * 6 );
}
*/

void clock_init( Window *window ) {
  window_layer = window_get_root_layer( window );
  GRect window_bounds = layer_get_bounds( window_layer );
  
  clockface_bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_CLOCKFACE );
  clockface_layer = bitmap_layer_create( window_bounds );
  bitmap_layer_set_bitmap( clockface_layer, clockface_bitmap );
  layer_add_child( window_layer, bitmap_layer_get_layer( clockface_layer ) );
  
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
  
  draw_clock();
}

void clock_deinit( void ) {
  tick_timer_service_unsubscribe();
  
  if (clockface_layer) bitmap_layer_destroy( clockface_layer );
  if (clockface_bitmap) gbitmap_destroy( clockface_bitmap );
}
