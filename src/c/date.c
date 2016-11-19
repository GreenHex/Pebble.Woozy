#include <pebble.h>
#include "date.h"

static GFont batt_font;
Layer *date_layer = 0;
TextLayer *date_text_layer = 0;

extern tm tm_time;

static void date_layer_update_proc( Layer *layer, GContext *ctx ) {
  GRect date_window_bounds = layer_get_bounds( layer );
  graphics_context_set_fill_color( ctx, GColorBlack );
  graphics_fill_rect( ctx, date_window_bounds, 0, GCornerNone );
  date_window_bounds = grect_inset( date_window_bounds, GEdgeInsets( 1, DATE_WINDOW_INSET, 1, DATE_WINDOW_INSET ) );
  graphics_context_set_stroke_color( ctx, GColorDarkGray );
  graphics_context_set_stroke_width( ctx, 1 );
  graphics_draw_round_rect( ctx, date_window_bounds, date_window_bounds.size.h / 2 );
}
 
static void date_text_layer_update_proc( Layer *layer, GContext *ctx ) {
  char date_str[] = "AAA, DD-MMM-YYYY";
  GRect date_window_bounds = grect_inset( layer_get_bounds( layer ), GEdgeInsets( DATE_STR_VERT_CORRECTION ) );
  graphics_context_set_text_color( ctx, GColorLightGray );
  strftime( date_str, sizeof( date_str ), "%a, %e-%b-%Y", &tm_time );
  graphics_draw_text( ctx, date_str, batt_font, date_window_bounds,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL );
}

void date_init( Layer* parent_layer )
{
  #if PBL_DISPLAY_WIDTH == 200
  batt_font = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_PRELUDE_MEDIUM_18 ) );
  #else
  batt_font = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_PRELUDE_MEDIUM_14 ) );  
  #endif
  
  GRect bounds = layer_get_bounds( parent_layer );
  
  bounds.origin.y += PBL_DISPLAY_WIDTH;
  bounds.size.h -= PBL_DISPLAY_WIDTH;
  
  date_layer = layer_create( bounds );
  layer_set_update_proc( date_layer, date_layer_update_proc );
  layer_add_child( parent_layer, date_layer );
  bounds.origin.y = 0;
  date_text_layer = text_layer_create( bounds );
  layer_set_update_proc( text_layer_get_layer( date_text_layer ), date_text_layer_update_proc );
  layer_add_child( date_layer, text_layer_get_layer( date_text_layer ) );
}

void date_deinit( void ) {
  fonts_unload_custom_font( batt_font );
  text_layer_destroy( date_text_layer );
  layer_destroy( date_layer );
}