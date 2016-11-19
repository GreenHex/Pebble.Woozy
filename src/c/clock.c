#include <pebble.h>
#include "global.h"
#include "clock.h"
#include "date.h"
#include "chime.h"

static Layer *window_layer = 0;
// analog clock
static BitmapLayer *hour_layer = 0;
static BitmapLayer *min_layer = 0;
static BitmapLayer *sec_layer = 0;
static GBitmap *bitmap_webos_clockface = 0;
static GBitmap *bitmap_webos_hour = 0;
// misc.
static bool hide_seconds_layer = true;
#ifndef SECONDS_ALWAYS_ON
static AppTimer *secs_display_apptimer = 0;
#endif

extern tm tm_time;

#ifndef SECONDS_ALWAYS_ON
static void start_seconds_display( AccelAxisType axis, int32_t direction );
#endif

// function is "adjusted"" for whole hours or minutes; "after" 9:00 AM or "upto" 9:00 AM.
// "after" includes the hour, "upto" excludes the hour.
bool is_X_in_range( int a, int b, int x ) { return ( ( b > a ) ? ( ( x >= a ) && ( x < b ) ) : ( ( x >= a ) || ( x < b ) ) ); };

void draw_clock( void ) {
  time_t now = time( NULL );
  tm_time = *localtime( &now ); // copy to global
  #ifndef SECONDS_ALWAYS_ON
  if ( persist_read_int( MESSAGE_KEY_ANALOG_SECONDS_DISPLAY_TIMEOUT_SECS ) ) accel_tap_service_subscribe( start_seconds_display );
  #endif
  layer_mark_dirty( window_layer );
}

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global
  
  // if (DEBUG) APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%d:%d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
 
  layer_set_hidden( bitmap_layer_get_layer( sec_layer ), hide_seconds_layer );
  layer_mark_dirty( window_layer );
  
  if ( ( units_changed & MINUTE_UNIT ) == MINUTE_UNIT ) do_chime( &tm_time );
}

#ifndef SECONDS_ALWAYS_ON
static void stop_seconds_display( void* data ) { // after timer elapses
  if ( secs_display_apptimer ) app_timer_cancel( secs_display_apptimer ); // just for fun.
  secs_display_apptimer = 0; // docs don't say if this is set to zero when timer expires. 

  hide_seconds_layer = true;
  
  tick_timer_service_subscribe( MINUTE_UNIT, handle_clock_tick );
}

static void start_seconds_display( AccelAxisType axis, int32_t direction ) {  
  if ( ! persist_read_int( MESSAGE_KEY_ANALOG_SECONDS_DISPLAY_TIMEOUT_SECS ) ) return;

  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );

  hide_seconds_layer = false;

  if ( secs_display_apptimer ) {
    app_timer_reschedule( secs_display_apptimer, (uint32_t) persist_read_int( MESSAGE_KEY_ANALOG_SECONDS_DISPLAY_TIMEOUT_SECS ) * 1000 );
  } else {
    secs_display_apptimer = app_timer_register( (uint32_t) persist_read_int( MESSAGE_KEY_ANALOG_SECONDS_DISPLAY_TIMEOUT_SECS ) * 1000,
                                               stop_seconds_display, 0 );
  }
}
#endif

static void hour_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  graphics_draw_bitmap_in_rect( ctx, bitmap_webos_clockface, layer_bounds );
  graphics_context_set_compositing_mode( ctx, GCompOpSet );
  uint32_t hour_angle = ( TRIG_MAX_ANGLE * ( ( ( tm_time.tm_hour % 12 ) * 6 ) + ( tm_time.tm_min / 10 ) ) ) / ( 12 * 6 );
  graphics_draw_rotated_bitmap( ctx, bitmap_webos_hour, GPoint( PBL_DISPLAY_WIDTH / 2, PBL_DISPLAY_WIDTH / 2 ),
                               hour_angle, GPoint( PBL_DISPLAY_WIDTH / 2, PBL_DISPLAY_WIDTH / 2 ) );
}

static void min_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  GPoint center_pt = grect_center_point( &layer_bounds );
  uint32_t min_angle = TRIG_MAX_ANGLE * tm_time.tm_min / 60;
  GPoint min_hand = (GPoint) {
    .x = ( sin_lookup( min_angle ) * MIN_HAND_LENGTH / TRIG_MAX_RATIO ) + center_pt.x,
    .y = ( -cos_lookup( min_angle ) * MIN_HAND_LENGTH / TRIG_MAX_RATIO ) + center_pt.y
  };
  graphics_context_set_stroke_color( ctx, GColorDarkGray );
  graphics_context_set_stroke_width( ctx, MIN_HAND_WIDTH );
  graphics_draw_line( ctx, GPoint( PBL_DISPLAY_WIDTH / 2, PBL_DISPLAY_WIDTH / 2 ), min_hand );
}
  
static void sec_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  GPoint center_pt = grect_center_point( &layer_bounds );
  int32_t sec_angle = TRIG_MAX_ANGLE * tm_time.tm_sec / 60;
  GPoint sec_dot_center = (GPoint) {
    .x = ( sin_lookup( sec_angle ) * SEC_DOT_DIST / TRIG_MAX_RATIO ) + center_pt.x,
    .y = ( -cos_lookup( sec_angle ) * SEC_DOT_DIST / TRIG_MAX_RATIO ) + center_pt.y
  };  
  graphics_context_set_fill_color( ctx, GColorOrange );
  graphics_fill_circle( ctx, sec_dot_center, SEC_DOT_RADIUS );
  graphics_context_set_stroke_color( ctx, GColorDarkCandyAppleRed );
  graphics_context_set_stroke_width( ctx, 1 );
  graphics_draw_circle( ctx, sec_dot_center, SEC_DOT_RADIUS );
}

void clock_init( Window *window ) {
  window_layer = window_get_root_layer( window );
  // GRect window_bounds = layer_get_bounds( window_layer );
  
  bitmap_webos_clockface = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_WEBOS_CLOCKFACE );
  bitmap_webos_hour = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_WEBOS_CLOCKFACE_HOUR );
  
  GRect clock_rect = GRect( 0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_WIDTH );
  hour_layer = bitmap_layer_create( clock_rect );
  bitmap_layer_set_bitmap( hour_layer, bitmap_webos_clockface );
  layer_add_child( window_layer, bitmap_layer_get_layer( hour_layer ) );
  layer_set_update_proc( bitmap_layer_get_layer( hour_layer ), hour_layer_update_proc );
  
  min_layer = bitmap_layer_create( clock_rect );
  layer_add_child( bitmap_layer_get_layer( hour_layer ), bitmap_layer_get_layer( min_layer ) );
  layer_set_update_proc( bitmap_layer_get_layer( min_layer ), min_layer_update_proc );
  
  sec_layer = bitmap_layer_create( clock_rect );
  layer_add_child( bitmap_layer_get_layer( min_layer ), bitmap_layer_get_layer( sec_layer ) );
  layer_set_update_proc( bitmap_layer_get_layer( sec_layer ), sec_layer_update_proc );
  
  #ifdef SECONDS_ALWAYS_ON
  layer_set_hidden( bitmap_layer_get_layer( sec_layer ), false );
  #else
  layer_set_hidden( bitmap_layer_get_layer( sec_layer ), true );
  #endif
  
  date_init( window_layer );  
 
  // subscriptions
  #ifdef SECONDS_ALWAYS_ON
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
  #else
  tick_timer_service_subscribe( MINUTE_UNIT, handle_clock_tick );
  #endif
  
  // show current time
  draw_clock();
}

void clock_deinit( void ) {
  #ifndef SECONDS_ALWAYS_ON
  if ( secs_display_apptimer ) app_timer_cancel( secs_display_apptimer );
  accel_tap_service_unsubscribe(); // are we over-unsubscribing?
  #endif
  date_deinit();
  tick_timer_service_unsubscribe();
  bitmap_layer_destroy( sec_layer );
  bitmap_layer_destroy( min_layer );
  bitmap_layer_destroy( hour_layer );
  gbitmap_destroy( bitmap_webos_hour );
  gbitmap_destroy( bitmap_webos_clockface );
}
