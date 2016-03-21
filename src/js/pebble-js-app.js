Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');

  // For display color selector or not.
  if(Pebble.getActiveWatchInfo) {
    console.log( 'PebbleKit getActiveWatchInfo available' );
  } else {
    console.log( 'PebbleKit getActiveWatchInfo unavailable' );
  }
} );

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message ' + JSON.stringify ( e.payload ) );

  var
    quotes = JSON.parse ( localStorage.quotes ),
    tosend = getQuoteIndex( 0, quotes.length ),
    dict = {};

  console.log ( "quotes: " + JSON.stringify ( quotes ) + " tosend: " + tosend );

  if ( quotes.length != 0 ) {
    dict['KEY_QUOTE']  = quotes[tosend][0];
    dict['KEY_AUTHOR'] = quotes[tosend][1];
  } else {
    dict['KEY_QUOTE']  = '';
    dict['KEY_AUTHOR'] = '';
  }

  Pebble.sendAppMessage( dict, function() {
    console.log ( 'Quote sent successfully.' + JSON.stringify ( dict ) );
  }, function () {
    console.log ( 'Send quote failed.' + JSON.stringify ( dict ) );
  });
});

Pebble.addEventListener('showConfiguration', function(e) {
  // var configurl = 'http://aocodermx.me/project/TimeAndQuotes/config/';
  var configurl = 'http://192.168.1.194:8000/config/';
  console.log('JavaScript configuration site will be loaded from: ' + configurl + " Config:" + JSON.stringify ( localStorage.quotes ) );
  Pebble.openURL( configurl );
} );

Pebble.addEventListener('webviewclosed', function(e) {
  var config = JSON.parse ( decodeURIComponent(e.response) );
  console.log('Configuration window returned: ' + JSON.stringify ( config ) );

  var dict = {
    'KEY_BACKGROUND_COLOR': config['bgcolor'],
    'KEY_TIME_24_HOURS'   : config['time24hours'],
    'KEY_SHOW_CALENDAR'   : config['showcalendar'],
    'KEY_CHANGE_QUOTE'    : config['changequote']
  };

  if ( config['quotes'].length != 0 ) {
    dict['KEY_QUOTE']  = config['quotes'][0][0];
    dict['KEY_AUTHOR'] = config['quotes'][0][1];
  }

  localStorage.quotes       = JSON.stringify ( config['quotes'] );
  localStorage.changequotes = parseInt ( config['changequotes'] );

  Pebble.sendAppMessage( dict, function() {
    console.log ( 'Configuration data sent successfully.' );
  }, function () {
    console.log ( 'Send configuration data failed.' );
  });
});

function getQuoteIndex(min, max) {
  if ( localStorage.changequotes == 1 ) {
    localStorage.qindex = Math.floor(Math.random() * (max - min)) + min;
    return parseInt ( localStorage.qindex );
  } else if ( localStorage.changequotes == 2 ) {
    if ( localStorage.qindex ) {
      localStorage.qindex = parseInt ( localStorage.qindex ) > max ? 0: parseInt( localStorage.qindex ) + 1;
    } else {
      localStorage.qindex = 0;
    }
    return max - parseInt ( localStorage.qindex );
  }
}
