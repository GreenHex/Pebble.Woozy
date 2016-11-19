module.exports = function( minified ) {
  var clayConfig = this;
  //  var _ = minified._;
  //  var $ = minified.$;
  //  var HTML = minified.HTML;

  function show( showItems ) {
    showItems.map( function( item ) {
      clayConfig.getItemByMessageKey( item ).show();
    });
  }

  function hide( hideItems ) {
    hideItems.map( function( item ) {
      clayConfig.getItemByMessageKey( item ).hide();
    });
  }
  
  var toggle_chime_settings_visibility = function() {
    var showItems;
    var hideItems;
    if ( this.get() == '0' ) { // no chime
      showItems = [ ];
      hideItems = [ 'CHIME_START_TIME', 'CHIME_END_TIME', 'CHIME_ON_DAYS', 'CHIME_OFFSET' ];
    } else { 
      showItems = [ 'CHIME_START_TIME', 'CHIME_END_TIME', 'CHIME_ON_DAYS', 'CHIME_OFFSET' ];
      hideItems = [ ];
    }
    show( showItems );
    hide( hideItems );
  };

  clayConfig.on( clayConfig.EVENTS.AFTER_BUILD, function() {
    var chimeSettings = clayConfig.getItemByMessageKey( 'CHIME_INTERVAL' );
    toggle_chime_settings_visibility.call( chimeSettings );
    chimeSettings.on( 'change', toggle_chime_settings_visibility );
    clayConfig.getItemByMessageKey( 'SHOW_BATTERY_GAUGE' ).hide();
  });
};
