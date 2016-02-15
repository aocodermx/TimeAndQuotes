/*
 * APP LIFECYCLE IMPLEMENTATION
 */

#include <pebble.h>
#include "window/QuoteWatchface.h"

static void app_init( ) {
  window_watchface_init();
}

static void app_deinit( ) {
  window_watchface_deinit();
}

/*
 * MAIN ROUTINE
 */
int main ( void ) {
  app_init ( );
  app_event_loop ( );
  app_deinit ( );
}
