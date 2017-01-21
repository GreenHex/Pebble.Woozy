//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//
// Fonts:
// https://fonts.google.com/specimen/Gloria+Hallelujah
// https://fonts.google.com/specimen/Schoolbell

#include <pebble.h>
#include "global.h"
#include "clock.h"
#include "animation.h"
#include "randomize_clockface.h"
#include "random.h"

#if defined( PBL_COLOR )
#define NUM_PBL_64_COLOURS 64

const uint32_t PBL_64_COLOURS[ NUM_PBL_64_COLOURS ] = {
  0x000000, 0xFFFFFF, 0xAAAAAA, 0x005555, 0xFFFFAA, 0xFFFF55, 0xFFAA55, 0xFF5500,
  0xFF0000, 0xFF0055, 0xFF5555, 0xFFAAAA, 0xFFFF00, 0xFFAA00, 0xAA5500, 0xAA5555,
  0xAA0000, 0xFF00AA, 0xFF55AA, 0xFFAAFF, 0x550000, 0xAA0055, 0xFF00FF, 0xFF55FF,
  0x550055, 0xAA00AA, 0xAA55AA, 0x000055, 0x5500AA, 0xAA00FF, 0xAA55FF, 0x0000AA,
  0x5500FF, 0x5555AA, 0x0055AA, 0x55AAAA, 0x55AA55, 0x00AA00, 0x00FF00, 0x55FF00,
  0xAAFF55, 0xAAFF00, 0x55AA00, 0x005500, 0x005555, 0xAAAA55, 0x555500, 0xAAAA00,
  0xAAFFAA, 0x55FF55, 0x00FF55, 0x00AA55, 0x00AAAA, 0x00AAFF, 0x0000FF, 0x5555FF,
  0xAAAAFF, 0x55FFAA, 0x00FFAA, 0x00FFFF, 0x55AAFF, 0x0055FF, 0x55FFFF, 0xAAFFFF
};
#endif /* PBL_COLOR */

extern BitmapLayer *clockface_layer;
extern Layer *snooze_layer;
extern Layer *digit_layer[];
extern Layer *hour_layer;
extern Layer *min_layer;
extern Layer *day_layer;
extern Layer *date_layer;
extern BitmapLayer *oops_layer;
extern GBitmap *oops_bitmap;
extern tm tm_time;

int rand_digit_order[ NUM_DIGITS ] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static Layer *window_layer = 0;
static GBitmap *clockface_bitmap = 0;
static char digit_str[] = "12";
static char day_str[] = "SUN";
static char date_str[] = "31";
static bool show_time = true;
static AppTimer *show_time_apptimer = 0;
static GPoint center_pt = { 0 };
static int16_t hour_hand_length = HOUR_HAND_LENGTH;
static int16_t min_hand_length = MIN_HAND_LENGTH;

static void show_time_timeout_proc( void* data );
static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed );
static void start_timer( AccelAxisType axis, int32_t direction );

static uint32_t const two_segments[] = { 200, 200, 200 };
VibePattern double_vibe_pattern = {
  .durations = two_segments,
  .num_segments = ARRAY_LENGTH( two_segments ),
};

static void draw_clock( void ) {
  time_t now = time( NULL );
  tm_time = *localtime( &now ); // copy to global
  
  // startup animation, need to do something about the repeating code
  uint32_t hour_angle = ( TRIG_MAX_ANGLE * ( ( ( tm_time.tm_hour % 12 ) * 6 ) + ( tm_time.tm_min / 10 ) ) ) / ( 12 * 6 );
  ( (HAND_LAYER_DATA *) layer_get_data( hour_layer ) )->home_rect.size = (GSize) {
    .w = ( sin_lookup( hour_angle ) * hour_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_WIDTH / 2,
    .h = ( -cos_lookup( hour_angle ) * hour_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_HEIGHT / 2
  };
  uint32_t min_angle = TRIG_MAX_ANGLE * tm_time.tm_min / 60;
  ( (HAND_LAYER_DATA *) layer_get_data( min_layer ) )->home_rect.size = (GSize) {
    .w = ( sin_lookup( min_angle ) * min_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_WIDTH / 2,
    .h = ( -cos_lookup( min_angle ) * min_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_HEIGHT / 2
  };
  start_animation( 0, 2000, AnimationCurveEaseInOut, true );

  show_time_apptimer = app_timer_register( 20 * 1000, show_time_timeout_proc, 0 );
  accel_tap_service_subscribe( start_timer );
}

static void handle_clock_tick( struct tm *tick_time, TimeUnits units_changed ) {
  tm_time = *tick_time; // copy to global  
  #ifdef DEBUG
  APP_LOG( APP_LOG_LEVEL_INFO, "clock.c: handle_clock_tick(): %d:%02d:%02d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
  #endif

  uint32_t hour_angle = ( TRIG_MAX_ANGLE * ( ( ( tm_time.tm_hour % 12 ) * 6 ) + ( tm_time.tm_min / 10 ) ) ) / ( 12 * 6 );
  ( (HAND_LAYER_DATA *) layer_get_data( hour_layer ) )->home_rect.size = (GSize) {
    .w = ( sin_lookup( hour_angle ) * hour_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_WIDTH / 2,
    .h = ( -cos_lookup( hour_angle ) * hour_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_HEIGHT / 2
  };
  uint32_t min_angle = TRIG_MAX_ANGLE * tm_time.tm_min / 60;
  ( (HAND_LAYER_DATA *) layer_get_data( min_layer ) )->home_rect.size = (GSize) {
    .w = ( sin_lookup( min_angle ) * min_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_WIDTH / 2,
    .h = ( -cos_lookup( min_angle ) * min_hand_length / TRIG_MAX_RATIO ) + PBL_DISPLAY_HEIGHT / 2
  };

  if ( show_time ) {
    ( (HAND_LAYER_DATA *) layer_get_data( hour_layer ) )->current_rect = ( (HAND_LAYER_DATA *) layer_get_data( hour_layer ) )->home_rect;
    ( (HAND_LAYER_DATA *) layer_get_data( min_layer ) )->current_rect = ( (HAND_LAYER_DATA *) layer_get_data( min_layer ) )->home_rect;
    layer_mark_dirty( window_layer );    
  } else {
    randomize_clockface();  
  }
  if ( ( units_changed & HOUR_UNIT ) && ( !quiet_time_is_active() ) ) vibes_enqueue_custom_pattern( double_vibe_pattern );
}

static void snooze_layer_update_proc( Layer *layer, GContext *ctx ) {
  if ( quiet_time_is_active() ) {
    GRect bounds = layer_get_bounds( layer );
    graphics_context_set_compositing_mode( ctx, GCompOpSet );
    GBitmap *snooze_bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_MOUSE );
    graphics_draw_bitmap_in_rect( ctx, snooze_bitmap, bounds );
    gbitmap_destroy( snooze_bitmap );
  }
}

static void make_outline( GContext *ctx, Layer *layer, GColor fgColour, GColor outlineColor ) {
  #if defined( PBL_COLOR )
  GRect frame = layer_get_frame( layer );
  GPoint origin = layer_convert_point_to_screen( layer, GPointZero );
  GBitmap *fb = graphics_capture_frame_buffer( ctx );
  
  for( int y = 1; y < frame.size.h - 1; y++ ) {
    GBitmapDataRowInfo r0 = gbitmap_get_data_row_info( fb, origin.y + y - 1 );
    GBitmapDataRowInfo r1 = gbitmap_get_data_row_info( fb, origin.y + y );
    GBitmapDataRowInfo r2 = gbitmap_get_data_row_info( fb, origin.y + y + 1 );
    
    for ( int x = 1; x < frame.size.w - 1; x++ ) {
      GColor c0r0 = (GColor) { .argb = r0.data[ origin.x + x - 1 ] };
      GColor c1r0 = (GColor) { .argb = r0.data[ origin.x + x ] };
      GColor c2r0 = (GColor) { .argb = r0.data[ origin.x + x + 1 ] };
      GColor c0r1 = (GColor) { .argb = r1.data[ origin.x + x - 1 ] };
      GColor c1r1 = (GColor) { .argb = r1.data[ origin.x + x ] };
      GColor c2r1 = (GColor) { .argb = r1.data[ origin.x + x + 1 ] };
      GColor c0r2 = (GColor) { .argb = r2.data[ origin.x + x - 1 ] };
      GColor c1r2 = (GColor) { .argb = r2.data[ origin.x + x ] };
      GColor c2r2 = (GColor) { .argb = r2.data[ origin.x + x + 1 ] };
     
      if ( gcolor_equal( c1r1, GColorWhite ) ) {
        if ( gcolor_equal( c0r0, fgColour ) || gcolor_equal( c1r0, fgColour ) || gcolor_equal( c2r0, fgColour ) ||
            gcolor_equal( c0r1, fgColour ) || gcolor_equal( c2r1, fgColour ) ||
            gcolor_equal( c0r2, fgColour ) || gcolor_equal( c1r2, fgColour ) || gcolor_equal( c2r2, fgColour ) ) {
          memset( &r1.data[ origin.x + x ], outlineColor.argb, 1 );
        }
      }
    } 
  }
  graphics_release_frame_buffer( ctx, fb );
  #endif
}

#define ALTERNATE_FONT


#if PBL_DISPLAY_WIDTH == 200
#define DIGIT_ALTERNATE_FONT RESOURCE_ID_FONT_ITIM_REGULAR_22 /* RESOURCE_ID_FONT_GLORIA_HALLELUJAH_32 */
#else
#define DIGIT_ALTERNATE_FONT RESOURCE_ID_FONT_ITIM_REGULAR_22 /* RESOURCE_ID_FONT_GLORIA_HALLELUJAH_22 */
#endif

// OR
// #define DIGIT_ALTERNATE_FONT RESOURCE_ID_FONT_ALADIN_REGULAR_22

static void digit_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  // graphics_context_set_fill_color( ctx, GColorLightGray );
  // graphics_fill_rect( ctx, layer_bounds, 0, GCornerNone );

  snprintf( digit_str, sizeof( digit_str ), "%u",  ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->digit );
  layer_bounds.origin.y -= DIGIT_TXT_VERT_ADJ;
  
  #ifdef ALTERNATE_FONT
  GFont font = fonts_load_custom_font( resource_get_handle( DIGIT_ALTERNATE_FONT ) );
  graphics_context_set_text_color( ctx, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorBlack ) );
  graphics_draw_text( ctx, digit_str, font, layer_bounds,
                     GTextOverflowModeTrailingEllipsis, ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->text_alignment, NULL );
  fonts_unload_custom_font( font );
  #else
  graphics_context_set_text_color( ctx, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorBlack ) );
  graphics_draw_text( ctx, digit_str, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ), layer_bounds,
                     GTextOverflowModeTrailingEllipsis, ( ( DIGIT_LAYER_DATA *) layer_get_data( layer ) )->text_alignment, NULL );
  #endif
  make_outline( ctx, layer, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorBlack ), GColorDarkGray );
}

static void day_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  strftime( day_str, sizeof( day_str ), "%a", &tm_time );
  layer_bounds.origin.y -= DIGIT_TXT_VERT_ADJ;
  
  #ifdef ALTERNATE_FONT
  GFont font = fonts_load_custom_font( resource_get_handle( DIGIT_ALTERNATE_FONT ) );
  graphics_context_set_text_color( ctx, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorWhite ) );
  graphics_draw_text( ctx, day_str, font, layer_bounds,
                     GTextOverflowModeTrailingEllipsis, ( ( DIGIT_LAYER_DATA *) layer_get_data( layer ) )->text_alignment, NULL );
  fonts_unload_custom_font( font );
  #else
  graphics_context_set_text_color( ctx, GColorBlack );
  graphics_draw_text( ctx, day_str, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ), layer_bounds,
                     GTextOverflowModeTrailingEllipsis, ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->text_alignment, NULL );
  #endif
  make_outline( ctx, layer, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorWhite ), GColorDarkGray );
}

static void date_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect layer_bounds = layer_get_bounds( layer );
  
  // tm_time.tm_mday = 28; 
  strftime( date_str, sizeof( date_str ), "%e", &tm_time );
  layer_bounds.origin.y -= DIGIT_TXT_VERT_ADJ;
  
  #ifdef ALTERNATE_FONT
  GFont font = fonts_load_custom_font( resource_get_handle( DIGIT_ALTERNATE_FONT ) );
  graphics_context_set_text_color( ctx, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorWhite ) );
  graphics_draw_text( ctx, date_str, font, layer_bounds,
                     GTextOverflowModeTrailingEllipsis, ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->text_alignment, NULL );
  fonts_unload_custom_font( font );
  #else
  graphics_context_set_text_color( ctx, GColorBlack ) );
  graphics_draw_text( ctx, date_str, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ), layer_bounds,
                     GTextOverflowModeTrailingEllipsis, ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->text_alignment, NULL );
  #endif
  make_outline( ctx, layer, PBL_IF_COLOR_ELSE( GColorFromHEX( ( (DIGIT_LAYER_DATA *) layer_get_data( layer ) )->colour ), GColorWhite ), GColorDarkGray );
}

static void hand_layer_update_proc( Layer *layer, GContext *ctx ) {
  HAND_LAYER_DATA *hand_layer_data = ( HAND_LAYER_DATA *) layer_get_data( layer );
  GPoint start_pt = GPoint( hand_layer_data->current_rect.origin.x, hand_layer_data->current_rect.origin.y );
  GPoint end_pt = GPoint( hand_layer_data->current_rect.size.w, hand_layer_data->current_rect.size.h );
  
  graphics_context_set_stroke_color( ctx, BG_BITMAP_BG_COLOUR );
  graphics_context_set_stroke_width( ctx, hand_layer_data->stroke_width + 2 );
  graphics_draw_line( ctx, start_pt, end_pt );
  
  graphics_context_set_stroke_color( ctx, PBL_IF_COLOR_ELSE( GColorFromHEX( hand_layer_data->colour ), GColorWhite ) );
  graphics_context_set_stroke_width( ctx, hand_layer_data->stroke_width );
  graphics_draw_line( ctx, start_pt, end_pt ); 
  
  graphics_context_set_fill_color( ctx, GColorBlack );
  graphics_fill_circle( ctx, start_pt, hand_layer_data->hole_radius );
}

static void oops_layer_update_proc( Layer *layer, GContext *ctx ) {
  graphics_context_set_compositing_mode( ctx, GCompOpSet );
  graphics_draw_bitmap_in_rect( ctx, oops_bitmap, layer_get_bounds( layer ) );
}

static void show_time_timeout_proc( void* data ) {
  show_time_apptimer = 0; // docs don't say if this is set to zero when timer expires. 
  show_time = false;
  tick_timer_service_subscribe( SECOND_UNIT, handle_clock_tick );
}

static void start_timer( AccelAxisType axis, int32_t direction ) {
  show_time = true;

  start_animation( 0, 1000, AnimationCurveEaseInOut, false ); // tick timer service unsubscribed here

  if ( show_time_apptimer ) {
    app_timer_reschedule( show_time_apptimer, SHOW_TIME_TIMER_TIMEOUT_MS );
  } else {
    show_time_apptimer = app_timer_register( SHOW_TIME_TIMER_TIMEOUT_MS, show_time_timeout_proc, 0 );
  }
}

static void unobstructed_change_proc( AnimationProgress progress, void *layer ) {
  GRect full_bounds = layer_get_bounds( layer );
  GRect unobstructed_bounds = layer_get_unobstructed_bounds( layer );
  
  int16_t full_height = full_bounds.size.h;
  int16_t unobstructed_height = unobstructed_bounds.size.h; /* - PBL_IF_COLOR_ELSE( 2, 2 ); */
  
  DIGIT_LAYER_DATA *snooze_layer_data = layer_get_data( snooze_layer );
  snooze_layer_data->current_rect.origin.y = ( (uint32_t) snooze_layer_data->home_rect.origin.y * unobstructed_height ) / full_height;
  layer_set_frame( snooze_layer, ( (DIGIT_LAYER_DATA *) layer_get_data( snooze_layer ) )->current_rect );
  
  DIGIT_LAYER_DATA *digit_layer_data = 0;
  for ( int i = 0; i < NUM_DIGITS; i++ ) {
    digit_layer_data = layer_get_data( digit_layer[i] );
    digit_layer_data->current_rect.origin.y = ( (uint32_t) digit_layer_data->home_rect.origin.y * unobstructed_height ) / full_height; 
    layer_set_frame( digit_layer[i], ( (DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) )->current_rect );
  }
  DIGIT_LAYER_DATA *day_layer_data = layer_get_data( day_layer );
  day_layer_data->current_rect.origin.y = ( (uint32_t) day_layer_data->home_rect.origin.y * unobstructed_height ) / full_height; 
  layer_set_frame( day_layer, ( (DIGIT_LAYER_DATA *) layer_get_data( day_layer ) )->current_rect );
  
  DIGIT_LAYER_DATA *date_layer_data = layer_get_data( date_layer );
  date_layer_data->current_rect.origin.y = ( (uint32_t) date_layer_data->home_rect.origin.y * unobstructed_height ) / full_height; 
  layer_set_frame( date_layer, ( (DIGIT_LAYER_DATA *) layer_get_data( date_layer ) )->current_rect );
  
  HAND_LAYER_DATA *hand_layer_data = 0; 
  hand_layer_data = (HAND_LAYER_DATA *) layer_get_data( hour_layer );
  hand_layer_data->current_rect.origin.y = ( hand_layer_data->home_rect.origin.y * unobstructed_height ) / full_height;
  hand_layer_data->current_rect.size.h = ( hand_layer_data->home_rect.size.h * unobstructed_height ) / full_height; 
  hour_hand_length = ( HOUR_HAND_LENGTH * unobstructed_height ) / full_height; 
  
  hand_layer_data = (HAND_LAYER_DATA *) layer_get_data( min_layer );
  hand_layer_data->current_rect.origin.y = ( hand_layer_data->home_rect.origin.y * unobstructed_height ) / full_height; 
  hand_layer_data->current_rect.size.h = ( hand_layer_data->home_rect.size.h * unobstructed_height ) / full_height;
  min_hand_length = ( MIN_HAND_LENGTH * unobstructed_height ) / full_height;
}

void clock_init( Window *window ) {
  window_layer = window_get_root_layer( window );
  GRect window_bounds = layer_get_bounds( window_layer );
  
  randomize( rand_digit_order, NUM_DIGITS );
  clockface_layer = bitmap_layer_create( window_bounds );
  layer_add_child( window_layer, bitmap_layer_get_layer( clockface_layer ) );
  #ifdef SHOW_BACKGROUND_BITMAP_IMAGE
  clockface_bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_CLOCKFACE );
  bitmap_layer_set_bitmap( clockface_layer, clockface_bitmap );
  #else
  window_set_background_color( window, GColorWhite );
  #endif
  GRect layer_uo_bounds = layer_get_unobstructed_bounds( bitmap_layer_get_layer( clockface_layer ) );
  
  snooze_layer = layer_create_with_data( GRect( layer_uo_bounds.size.w/2 - 20/2, layer_uo_bounds.size.h/2 - 20/2, 20, 20 ),
                                        sizeof( DIGIT_LAYER_DATA ) );
  *(DIGIT_LAYER_DATA *) layer_get_data( snooze_layer ) = (DIGIT_LAYER_DATA) {
    .home_rect = SNOOZE_LAYER_FRAME,
    .current_rect = GRect( layer_uo_bounds.size.w/2 - 20/2,
                          layer_uo_bounds.size.h/2 - 20/2,
                          20, 20 )
  };
  layer_set_update_proc( snooze_layer, snooze_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), snooze_layer );
  
  GRect digit_layer_frame_home_rect;
  GRect digit_layer_frame_current_rect;
  // DIGIT_LAYER_DATA *digit_layer_data = 0;
  for ( int i = 0; i < NUM_DIGITS; i ++ ) {
    digit_layer_frame_home_rect = GRect( DIGIT_LOCATIONS.points[ rand_digit_order[i] ].x,
                                        DIGIT_LOCATIONS.points[ rand_digit_order[i] ].y,
                                        DIGIT_RECT_SIZE_W, DIGIT_RECT_SIZE_H ); 
    digit_layer_frame_current_rect = GRect( layer_uo_bounds.size.w/2 - DIGIT_RECT_SIZE_W/2,
                                           layer_uo_bounds.size.h/2 - DIGIT_RECT_SIZE_W/2,
                                           DIGIT_RECT_SIZE_W, DIGIT_RECT_SIZE_H );
    digit_layer[i] = layer_create_with_data( digit_layer_frame_current_rect, sizeof( DIGIT_LAYER_DATA ) );
    *(DIGIT_LAYER_DATA *) layer_get_data( digit_layer[i] ) = (DIGIT_LAYER_DATA) {
      .digit = i + 1,
      .colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF ),
      .text_alignment = GTextAlignmentCenter,
      .home_rect = digit_layer_frame_home_rect,
      .current_rect = digit_layer_frame_current_rect
    };
    layer_set_update_proc( digit_layer[i], digit_layer_update_proc );
    layer_add_child( bitmap_layer_get_layer( clockface_layer ), digit_layer[i] );
  }
  
  GRect day_layer_frame_home_rect = DAY_RECT;
  GRect day_layer_frame_current_rect = GRect( layer_uo_bounds.size.w/2 -  DAY_RECT.size.w/2,
                                             layer_uo_bounds.size.h/2 - DAY_RECT.size.h/2,
                                             DAY_RECT.size.w, DAY_RECT.size.h );
  day_layer = layer_create_with_data( day_layer_frame_current_rect, sizeof( DIGIT_LAYER_DATA ) );
  *(DIGIT_LAYER_DATA *) layer_get_data( day_layer ) = (DIGIT_LAYER_DATA) {
    .colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF ),
    .text_alignment = GTextAlignmentCenter,
    .home_rect = day_layer_frame_home_rect,
    .current_rect = day_layer_frame_current_rect
  };
  layer_set_update_proc( day_layer, day_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), day_layer );

  GRect date_layer_frame_home_rect = DATE_RECT;
  GRect date_layer_frame_current_rect = GRect( layer_uo_bounds.size.w/2 -  DATE_RECT.size.w/2,
                                              layer_uo_bounds.size.h/2 - DATE_RECT.size.h/2,
                                              DATE_RECT.size.w, DATE_RECT.size.h );
  date_layer = layer_create_with_data( date_layer_frame_current_rect, sizeof( DIGIT_LAYER_DATA ) );
  *(DIGIT_LAYER_DATA *) layer_get_data( date_layer ) = (DIGIT_LAYER_DATA) {
    .colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF ),
    .text_alignment = GTextAlignmentCenter,
    .home_rect = date_layer_frame_home_rect,
    .current_rect = date_layer_frame_current_rect
  };
  layer_set_update_proc( date_layer, date_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), date_layer );

  hour_layer = layer_create_with_data( window_bounds, sizeof( HAND_LAYER_DATA ) );
  *(HAND_LAYER_DATA *) layer_get_data( hour_layer ) = (HAND_LAYER_DATA) { 
    .colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF ),
    .stroke_width = HOUR_HAND_THK,
    .hole_radius = 2,
    .layer_frame = window_bounds,
    .home_rect = HOUR_RECT,
    .current_rect = HOUR_RECT
  };
  layer_set_update_proc( hour_layer, hand_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), hour_layer );

  min_layer = layer_create_with_data( window_bounds, sizeof( HAND_LAYER_DATA ) );
  *(HAND_LAYER_DATA *) layer_get_data( min_layer ) = (HAND_LAYER_DATA) {
    .colour = PBL_IF_COLOR_ELSE( PBL_64_COLOURS[ rand() % ( NUM_PBL_64_COLOURS - 3 ) + 3 ], 0xFFFFFF ),
    .stroke_width = MIN_HAND_THK,
    .hole_radius = 1,
    .layer_frame = window_bounds,
    .home_rect = MIN_RECT,
    .current_rect = MIN_RECT,
  };
  layer_set_update_proc( min_layer, hand_layer_update_proc );
  layer_add_child( bitmap_layer_get_layer( clockface_layer ), min_layer );
  
  oops_bitmap = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_ICON_OOPS );
  oops_layer = bitmap_layer_create( OOPS_RECT );
  layer_set_update_proc( bitmap_layer_get_layer( oops_layer ), oops_layer_update_proc );
  layer_add_child( min_layer, bitmap_layer_get_layer( oops_layer ) );
  layer_set_hidden( bitmap_layer_get_layer( oops_layer ), true );
  
  draw_clock();
  
  unobstructed_area_service_subscribe( (UnobstructedAreaHandlers) { .change = unobstructed_change_proc }, window_layer );
}

void clock_deinit( void ) {
  if ( show_time_apptimer ) app_timer_cancel( show_time_apptimer );
  unobstructed_area_service_unsubscribe();
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();

  if ( oops_bitmap ) gbitmap_destroy( oops_bitmap );
  if ( oops_layer ) bitmap_layer_destroy( oops_layer );
  if ( date_layer ) layer_destroy( date_layer );
  if ( day_layer ) layer_destroy( day_layer );
  if ( min_layer ) layer_destroy( min_layer );
  if ( hour_layer ) layer_destroy( hour_layer );
  for ( int i = 0; i < NUM_DIGITS; i ++ ) {
    if ( digit_layer[i] ) layer_destroy( digit_layer[i] );
  }
  if ( snooze_layer ) layer_destroy( snooze_layer );
  if ( clockface_layer ) bitmap_layer_destroy( clockface_layer );
  if ( clockface_bitmap ) gbitmap_destroy( clockface_bitmap );
}
