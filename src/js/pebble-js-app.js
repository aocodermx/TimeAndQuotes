Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');

  // For display color selector or not.
  if(Pebble.getActiveWatchInfo) {
    console.log( 'PebbleKit getActiveWatchInfo available' );
  } else {
    console.log( 'PebbleKit getActiveWatchInfo unavailable' );
  }
} );

Pebble.addEventListener('showConfiguration', function(e) {
  var configurl = 'http://192.168.1.194:8000/config/';
  console.log('JavaScript configuration loaded');
  Pebble.openURL( configurl );
} );
