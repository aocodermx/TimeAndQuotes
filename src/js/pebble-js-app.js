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
    tosend = getRandomInt( 0, quotes.length ),
    dict = {
      'KEY_QUOTE' : quotes[tosend][0],
      'KEY_AUTHOR': quotes[tosend][1]
    };

  Pebble.sendAppMessage( dict, function() {
    console.log ( 'Quote sent successfully.' + JSON.stringify ( dict ) );
  }, function () {
    console.log ( 'Send quote failed.' + JSON.stringify ( dict ) );
  });
});

Pebble.addEventListener('showConfiguration', function(e) {
  console.log('JavaScript configuration site will be loaded');
  var configurl = 'http://aocodermx.me/project/TimeAndQuotes/config/';
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

  localStorage.quotes = JSON.stringify ( config['quotes'] );

  Pebble.sendAppMessage( dict, function() {
    console.log ( 'Configuration data sent successfully.' );
  }, function () {
    console.log ( 'Send configuration data failed.' );
  });
});

function getRandomInt(min, max) {
  return Math.floor(Math.random() * (max - min)) + min;
}
