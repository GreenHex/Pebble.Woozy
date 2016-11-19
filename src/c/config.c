#include <pebble.h>
#include "global.h"
#include "config.h"
#include "clock.h"

#define NUM_INT_MESSAGE_KEYS 6
#define NUM_BOOL_CHKBOX_MESSAGE_KEYS 1

typedef struct {
  uint32_t keyID;
  int numChkBoxItems;
} BOOL_CHKBOX_KEY;

static int stringToInt( char *str );

void handle_config_message( DictionaryIterator *iterator ) {
  if ( !iterator ) return;

  // this is all specific to pebble/clay, which always returns "data" for integers
  // could be const arrays..., but we don't want to lose automatic key assignment...
  // in all-caps, pretending to be constant values. This should be outside the 
  // function, but since these are variables, tough luck.
  uint32_t INT_MESSAGE_KEYS[ NUM_INT_MESSAGE_KEYS ] = {
    // Clock
    MESSAGE_KEY_ANALOG_SECONDS_DISPLAY_TIMEOUT_SECS,
    MESSAGE_KEY_SHOW_BATTERY_GAUGE,
    // Chime
    MESSAGE_KEY_CHIME_INTERVAL,
    MESSAGE_KEY_CHIME_START_TIME,
    MESSAGE_KEY_CHIME_END_TIME,
    MESSAGE_KEY_CHIME_OFFSET
  };

  Tuple *p_tuple = 0;

  for ( int i = 0 ; i < NUM_INT_MESSAGE_KEYS; i++ ) {
    if ( ( p_tuple = dict_find( iterator, INT_MESSAGE_KEYS[ i ] ) ) ) {
      persist_write_int( INT_MESSAGE_KEYS[ i ], stringToInt( (char*) p_tuple->value->data ) );
    }
  }

  BOOL_CHKBOX_KEY BOOL_CHKBOX_MESSAGE_KEYS[ NUM_BOOL_CHKBOX_MESSAGE_KEYS ] = {
    { .keyID = MESSAGE_KEY_CHIME_ON_DAYS, .numChkBoxItems = 7 }
  };

  for ( int i = 0 ; i < NUM_BOOL_CHKBOX_MESSAGE_KEYS; i++ ) {
    for ( int j = 0 ; j < BOOL_CHKBOX_MESSAGE_KEYS[i].numChkBoxItems ; j++ ) {
      if ( ( p_tuple = dict_find( iterator, BOOL_CHKBOX_MESSAGE_KEYS[i].keyID + j ) ) ) {
        persist_write_bool( BOOL_CHKBOX_MESSAGE_KEYS[i].keyID + j, ( ( p_tuple->value->uint8 == 't' ) ||
                                                                    ( p_tuple->value->uint8 == 'T' ) ||
                                                                    ( p_tuple->value->uint8 == '1' ) ) );
        // if (DEBUG) APP_LOG( APP_LOG_LEVEL_INFO, "config.c: handle_config_message(): %ld: %d %d", 
        //                   BOOL_CHKBOX_MESSAGE_KEYS[i].keyID + j, persist_read_bool( BOOL_CHKBOX_MESSAGE_KEYS[i].keyID + j ), p_tuple->value->uint8 );
      }
    }
  }
  draw_clock();
}

// need to do something about this, at some point this will break
static int stringToInt( char *str ) {
  int i = 0;
  int sum = 0;
  while ( str[i] != '\0' ) {
    if ( str[i]< 48 || str[i] > 57 ) {
      if (DEBUG) APP_LOG( APP_LOG_LEVEL_ERROR, "config.c: stringToInt(): Unable to convert it into integer." );
      return 0;
    }
    else {
      sum = sum * 10 + ( str[i] - 48 );
      i++;
    }
  }
  return sum;
}