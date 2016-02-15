#include <pebble.h>
#include "QuoteWatchface.h"
#include "Calendar.h"


static Window    *s_quotes_window;
static TextLayer *s_quote_layer;
static TextLayer *s_quote_author_layer;
static TextLayer *s_time_layer;


static void main_window_load      ( Window * );
static void main_window_unload    ( Window * );
static void main_window_appear    ( Window * );
static void main_window_disappear ( Window * );
static void update_quote  ( );
static void update_time   ( );
static void tick_handler  ( struct tm *, TimeUnits );
static void tap_handler   ( AccelAxisType , int32_t );


/*
 *  IMPLEMENTATION FOR QUOTEWATCHFACE WINDOW
 */


void window_watchface_init() {
  s_quotes_window = window_create();

  tick_timer_service_subscribe ( MINUTE_UNIT , tick_handler );
  window_set_window_handlers ( s_quotes_window, ( WindowHandlers ) {
    .load   = main_window_load,
    .unload = main_window_unload,
    .appear = main_window_appear,
    .disappear = main_window_disappear
  } );

  window_stack_push ( s_quotes_window, true );
}


void window_watchface_deinit() {
  window_destroy ( s_quotes_window );
}


static void main_window_load ( Window *window ) {
  Layer *window_layer = window_get_root_layer ( window );
  GRect bounds        = layer_get_bounds      ( window_layer );

  // Create quote TextLayer and add it to Window hierarchy
  s_quote_layer = text_layer_create ( GRect ( 0, 0, bounds.size.w, bounds.size.h - BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color ( s_quote_layer, GColorBlack );
  text_layer_set_text_color       ( s_quote_layer, GColorClear );
  text_layer_set_text_alignment   ( s_quote_layer, GTextAlignmentLeft );
  layer_add_child ( window_get_root_layer ( window ), text_layer_get_layer ( s_quote_layer ) );

  // Create quote author TextLayer and add it to Window hierarchy
  s_quote_author_layer = text_layer_create ( GRect ( 0, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR, bounds.size.w, BAR_AUTHOR ) );
  text_layer_set_background_color ( s_quote_author_layer, GColorBlack );
  text_layer_set_text_color       ( s_quote_author_layer, GColorClear );
  text_layer_set_text_alignment   ( s_quote_author_layer, GTextAlignmentRight );
  text_layer_set_font             ( s_quote_author_layer, fonts_get_system_font ( FONT_KEY_GOTHIC_14 ) );
  layer_add_child ( window_get_root_layer ( window ), text_layer_get_layer ( s_quote_author_layer ) );

  // Create time TextLayer and add it to Windows hierarchy
  s_time_layer = text_layer_create ( GRect ( 0, bounds.size.h - BAR_DATETIME_HEIGTH, bounds.size.w, BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color ( s_time_layer, GColorClear );
  text_layer_set_text_color       ( s_time_layer, GColorBlack );
  text_layer_set_text_alignment   ( s_time_layer, GTextAlignmentCenter );
  text_layer_set_font             ( s_time_layer, fonts_get_system_font( FONT_KEY_DROID_SERIF_28_BOLD ) );
  layer_add_child ( window_get_root_layer ( window ), text_layer_get_layer ( s_time_layer ) );

  update_quote ( );
  update_time();

  // APP_LOG( APP_LOG_LEVEL_INFO, "TimeUnits values: SECOND %d, MINUTE %d, HOUR %d, DAY %d, MONTH %d, YEAR %d ", SECOND_UNIT, MINUTE_UNIT, HOUR_UNIT, DAY_UNIT, MONTH_UNIT, YEAR_UNIT );
}

static void main_window_unload ( Window *window ) {
  text_layer_destroy ( s_quote_layer );
  text_layer_destroy ( s_quote_author_layer );
  text_layer_destroy ( s_time_layer );
}

static void main_window_appear ( Window *window ) {
  accel_tap_service_subscribe  ( tap_handler );
}

static void main_window_disappear ( Window *window ){
  accel_tap_service_unsubscribe ( );
}

static void update_quote ( ) {
  GFont quote_font;

  const char * quote  = "Tanto si crees que puedes hacerlo, como si no, en los dos casos tienes razón.";
  const char * author =  "Henry Ford";

  int quote_len = strlen( quote );

  // Four levels of font sizes, select the right one
       if ( quote_len < 20 )                    quote_font = fonts_get_system_font ( FONT_KEY_BITHAM_30_BLACK );
  else if ( quote_len > 20 && quote_len < 50  ) quote_font = fonts_get_system_font ( FONT_KEY_GOTHIC_28 );
  else if ( quote_len > 50 && quote_len < 60  ) quote_font = fonts_get_system_font ( FONT_KEY_GOTHIC_24 );
  else if ( quote_len > 60 && quote_len < 120 ) quote_font = fonts_get_system_font ( FONT_KEY_GOTHIC_18 );
  else      quote_font = fonts_get_system_font ( FONT_KEY_GOTHIC_14 );

  text_layer_set_font ( s_quote_layer, quote_font );
  text_layer_set_text ( s_quote_layer, quote );
  text_layer_set_text ( s_quote_author_layer, author);
}

static void update_time ( ) {
  static char buffer[] = "00:00 XX";
  time_t temp          = time      ( NULL );
  struct tm *tick_time = localtime ( &temp );

  if ( clock_is_24h_style( ) == true )
    strftime ( buffer, sizeof( "00:00 XX" ), "%H:%M", tick_time );
  else
    strftime ( buffer, sizeof( "00:00 XX" ), "%I:%M %p", tick_time );

  text_layer_set_text ( s_time_layer, buffer );
}

static void tick_handler ( struct tm *tick_time, TimeUnits units_changed ) {
  update_time ( );
  update_quote ( );
  // Update quote from configuration

  switch ( units_changed ) {
    case MINUTE_UNIT:
      APP_LOG( APP_LOG_LEVEL_INFO, "Unit changed: MINUTE(%d)", units_changed );
      break;
    case HOUR_UNIT:
      APP_LOG( APP_LOG_LEVEL_INFO, "Unit changed: HOUR(%d)", units_changed );
      break;
    case DAY_UNIT:
      //update_quote ( true );
      APP_LOG( APP_LOG_LEVEL_INFO, "Unit changed: DAY(%d)", units_changed );
      break;
    case MONTH_UNIT:
      APP_LOG( APP_LOG_LEVEL_INFO, "Unit changed: MONTH(%d)", units_changed );
      break;
    case YEAR_UNIT:
      APP_LOG( APP_LOG_LEVEL_INFO, "Unit changed: YEAR(%d)", units_changed );
      break;

    default:
      APP_LOG( APP_LOG_LEVEL_INFO, "Unit changed");
      break;
  }

  APP_LOG( APP_LOG_LEVEL_INFO, "Units Changed: %d", units_changed );
}

static void tap_handler ( AccelAxisType axis, int32_t direction ) {
  window_calendar_init();
}
