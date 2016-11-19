#pragma once

#if PBL_DISPLAY_WIDTH == 200
#define DATE_WINDOW_INSET 7
#define DATE_STR_VERT_CORRECTION 2
#else
#define DATE_WINDOW_INSET 5
#define DATE_STR_VERT_CORRECTION 3
#endif

void date_init( Layer* parent_layer );
void date_deinit( void );