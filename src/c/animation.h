//
// Copyright (c) 2016 Vinodh Kumar M. <GreenHex@gmail.com>
//

#pragma once
#include <pebble.h>
#include "global.h"

void start_second_animation( void *data );
void start_animation( int delay_ms, int duration_ms, AnimationCurve anim_curve, bool do_second_animation );
