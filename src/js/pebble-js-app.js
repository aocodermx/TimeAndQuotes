Pebble.addEventListener('ready', function(e) {
  console.log ( 'PebbleKit JS ready!' );

  if ( !localStorage.quotes )
    localStorage.quotes = JSON.stringify ( new Array ( ) );
} );

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message ' + JSON.stringify ( e.payload ) );

  var
    quotes = JSON.parse ( localStorage.quotes ),
    tosend = getQuoteIndex( 0, quotes.length ),
    dict = {};

  console.log ( "quotes: " + JSON.stringify ( quotes ) + " tosend: " + tosend );

  if ( quotes.length > 0 ) {
    dict['KEY_QUOTE']  = quotes[tosend][0];
    dict['KEY_AUTHOR'] = quotes[tosend][1];
  } else {
    dict['KEY_QUOTE']  = 'Visit your app settings to add quotes.';
    dict['KEY_AUTHOR'] = 'AOCoderMX';
  }

  Pebble.sendAppMessage( dict, function() {
    console.log ( 'Quote sent successfully.' + JSON.stringify ( dict ) );
  }, function () {
    console.log ( 'Send quote failed.' + JSON.stringify ( dict ) );
  });
});

Pebble.addEventListener('showConfiguration', function(e) {
  var
    watch_platform = Pebble.getActiveWatchInfo ? Pebble.getActiveWatchInfo().platform : "aplite",
    configurl = 'http://aocodermx.me/projects/TimeAndQuotes/config/v1.0/?watch_platform=' +
        encodeURIComponent ( watch_platform ) +
        '&quotes=' + encodeURIComponent ( localStorage.quotes );
  //var configurl = 'http://192.168.1.194:8000/config/?watch_platform=' + encodeURIComponent ( watch_platform ) + '&quotes=' + encodeURIComponent ( localStorage.quotes );

  console.log('JavaScript configuration site will be loaded from: ' + configurl + "     Saved Quotes: " + JSON.stringify ( localStorage.quotes ) );
  Pebble.openURL( configurl );
} );

Pebble.addEventListener('webviewclosed', function(e) {
  if ( e.response ) {
    var config = JSON.parse ( decodeURIComponent(e.response) );
    console.log('Configuration window returned: ' + JSON.stringify ( config ) );

    var dict = {
      'KEY_BACKGROUND_COLOR': parseInt ( config['bgcolor'], 16 ),
      'KEY_TIME_24_HOURS'   : config['time24hours'],
      'KEY_SHOW_CALENDAR'   : config['showcalendar'],
      'KEY_CHANGE_QUOTE'    : config['changequote'],
      'KEY_SHOW_BATTERY'    : config['showbattery']
    };

    if ( config['quotes'].length != 0 ) {
      dict['KEY_QUOTE']  = config['quotes'][0][0];
      dict['KEY_AUTHOR'] = config['quotes'][0][1];
    }

    localStorage.quotes       = JSON.stringify ( config['quotes'] );
    localStorage.changequotes = parseInt ( config['changequotes'] );

    console.log ( 'Preparing to send ... ' + JSON.stringify ( dict ) );

    Pebble.sendAppMessage( dict, function() {
      console.log ( 'Configuration data sent successfully.' );
    }, function () {
      console.log ( 'Send configuration data failed.' );
    });
  } else {
    console.log ( "Response not received." );
  }
});

function getQuoteIndex(min, max) {
  if ( max == 0 ) return -1;

  // Randomly
  if ( localStorage.changequotes == 1 ) {
    localStorage.qindex = Math.floor ( Math.random ( ) * ( max - min ) ) + min;
    return parseInt ( localStorage.qindex );

  // Secuentially
  } else if ( localStorage.changequotes == 2 ) {
    if ( localStorage.qindex ) {
      localStorage.qindex = parseInt ( localStorage.qindex ) < 1 ? max - 1 : parseInt( localStorage.qindex ) - 1;
    } else {
      localStorage.qindex = max - 1;
    }
    return parseInt ( localStorage.qindex );
  }
}
