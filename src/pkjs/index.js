var Clay = require( 'pebble-clay' );
var clayConfig = require( './config' );
var clayManipulator = require( './config_manipulator' );
var clay = new Clay( clayConfig, clayManipulator, { autoHandleEvents: false } );
var MESSAGE_KEYS = require( 'message_keys' );
// var COLOURS = require( './colours' );

var DEBUG = 0;

var CMD_TYPES = {
  CMD_UNDEFINED : 0,
  CMD_WEATHER : 1,
  CMD_STOCKS : 2,
  CMD_CONFIG : 3
};
Object.freeze( CMD_TYPES );

// clay should be able to give these, but settings are stored locally to enable sending configuration settings on call.
// But need to hook into clay, to get rid of all this.
var local_config_settings = [
  // clock
  MESSAGE_KEYS.ANALOG_SECONDS_DISPLAY_TIMEOUT_SECS,
  MESSAGE_KEYS.SHOW_BATTERY_GAUGE,
  // chime
  MESSAGE_KEYS.CHIME_INTERVAL,
  MESSAGE_KEYS.CHIME_START_TIME,
  MESSAGE_KEYS.CHIME_END_TIME,
  MESSAGE_KEYS.CHIME_ON_DAYS, MESSAGE_KEYS.CHIME_ON_DAYS + 1, MESSAGE_KEYS.CHIME_ON_DAYS + 2,
  MESSAGE_KEYS.CHIME_ON_DAYS + 3, MESSAGE_KEYS.CHIME_ON_DAYS + 4, MESSAGE_KEYS.CHIME_ON_DAYS + 5,
  MESSAGE_KEYS.CHIME_ON_DAYS + 6,
  MESSAGE_KEYS.CHIME_OFFSET,
];

function sendDictionaryToPebble( dictionary ) {
  Pebble.sendAppMessage( dictionary,
                        function(e) {
                          if (DEBUG) console.log( "index.js: sendDictionaryToPebble(): Message sent to Pebble successfully. " + JSON.stringify( dictionary ) );
                        },
                        function(e) {
                          if (DEBUG) console.log( "index.js: sendDictionaryToPebble(): Error sending message to Pebble. " + JSON.stringify( e ) );
                        });
}

/// Send configuration data
function sendConfig() {
  var dictionary = {};
  local_config_settings.map( function ( item ) {
    dictionary[item] = localStorage.getItem( item );
    if (DEBUG) console.log( "index.js: sendConfig(): " + item + ": " + dictionary[item] );
  });
  sendDictionaryToPebble( dictionary );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
                        function(e) {
                          if (DEBUG) console.log( "index.js: addEventListener( ready ): PebbleKit JS ready." );
                          sendConfig();
                        });

// Listen for when an AppMessage is received
Pebble.addEventListener( 'appmessage',
                        function(e) {
                          if (DEBUG) console.log( "index.js: addEventListener( appmessage ): AppMessage received: " + JSON.stringify( e.payload ) );
                          var dict = e.payload;
                          if( dict.REQUEST ) {
                            sendConfig();
                          }
                        });

/////// Pebble/clay stuff
Pebble.addEventListener( 'showConfiguration', function( e ) {
  Pebble.openURL( clay.generateUrl() );
});

Pebble.addEventListener( 'webviewclosed', function( e ) {
  if ( e && !e.response ) {
    return;
  }
  var dictionary = clay.getSettings( e.response );
  if (DEBUG) console.log( "index.js/clay: " + JSON.stringify( dictionary ) );
  // save for later...
  local_config_settings.map( function( item ) {
    localStorage.setItem( item, dictionary[ item ] );
    // if (DEBUG) console.log( "index.js/clay: " + item + " " + localStorage.getItem( item ) );
  });
  sendConfig();
});
