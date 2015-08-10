#include <pebble.h>

#define BAR_DATETIME_HEIGTH 35
#define BAR_AUTHOR 20

static Window    *s_main_window;
static TextLayer *s_quote_layer;
static TextLayer *s_quote_author_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;

static void print_quote( const char * quote, const char * author ) {
  // Count quote lenght
  int quote_len = strlen( quote );
  GFont quote_font;

  // Four levels of font sizes, select the right one
  if( quote_len < 20 ) {
    quote_font = fonts_get_system_font( FONT_KEY_BITHAM_30_BLACK );
  } else if ( quote_len > 20 && quote_len < 50 ) {
    quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_28 );
  } else if ( quote_len > 50 && quote_len < 60 ) {
    quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_24 );
  } else if ( quote_len > 60 && quote_len < 120 ) {
    quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_18 );
  } else {
    quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_14 );
  }

  // Set the selected font
  text_layer_set_font( s_quote_layer, quote_font );

  // Print the quote and author
  text_layer_set_text( s_quote_layer, quote );
  text_layer_set_text( s_quote_author_layer, author);
}

static void main_window_load( Window *window ) {
  // Get the root layer and their bounds
  Layer *window_layer = window_get_root_layer( window );
  GRect bounds = layer_get_bounds( window_layer );

  // Create quote TextLayer and add it to Window hierarchy
  s_quote_layer = text_layer_create( GRect( 0, 0, bounds.size.w, bounds.size.h - BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color( s_quote_layer, GColorBlack );
  text_layer_set_text_color( s_quote_layer, GColorClear );
  // text_layer_set_font( s_quote_layer, fonts_get_system_font( FONT_KEY_GOTHIC_28 ) );
  text_layer_set_text_alignment( s_quote_layer, GTextAlignmentLeft );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( s_quote_layer ) );

  // Create quote author TextLayer and add it to Window hierarchy
  s_quote_author_layer = text_layer_create( GRect( 0, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR, bounds.size.w, BAR_AUTHOR ) );
  text_layer_set_background_color( s_quote_author_layer, GColorBlack );
  text_layer_set_text_color( s_quote_author_layer, GColorClear );
  text_layer_set_font( s_quote_author_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
  text_layer_set_text_alignment( s_quote_author_layer, GTextAlignmentRight );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( s_quote_author_layer ) );

  print_quote( "Porque morir no duele, lo que duele es el olvido.", "Subcomandante Marcos" );

  // Create date TextLayer
  s_date_layer = text_layer_create( GRect( 0, bounds.size.h - BAR_DATETIME_HEIGTH, bounds.size.w * 0.4, BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color( s_date_layer, GColorClear );
  text_layer_set_text_color( s_date_layer, GColorBlack );
  text_layer_set_text( s_date_layer, "Day Month" );
  text_layer_set_font( s_date_layer, fonts_get_system_font( FONT_KEY_GOTHIC_24_BOLD ) );
  text_layer_set_text_alignment( s_date_layer, GTextAlignmentCenter );
  layer_add_child( window_get_root_layer( window ) , text_layer_get_layer( s_date_layer ) );

  // Create time TextLayer and add it to Windows hierarchy
  s_time_layer = text_layer_create( GRect( bounds.size.w * 0.4, bounds.size.h - BAR_DATETIME_HEIGTH, bounds.size.w * 0.6, BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color( s_time_layer, GColorClear );
  text_layer_set_text_color( s_time_layer, GColorBlack );
  text_layer_set_text( s_time_layer, "00:00" );
  text_layer_set_font( s_time_layer, fonts_get_system_font( FONT_KEY_DROID_SERIF_28_BOLD ) );
  text_layer_set_text_alignment( s_time_layer, GTextAlignmentCenter );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( s_time_layer ) );
}

static void main_window_unload( Window *window ) {
  // Destroy TextLayer
  text_layer_destroy( s_quote_layer );
  text_layer_destroy( s_quote_author_layer );
  text_layer_destroy( s_date_layer );
  text_layer_destroy( s_time_layer );
}

static void update_time() {
  // Get a tm structure
  time_t temp = time( NULL );
  struct tm *tick_time = localtime( &temp );

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char date[] = "00 Month";

  // Write the current hours and minutes into the buffer
  if( clock_is_24h_style() == true ) {
    strftime( buffer, sizeof( "00:00" ), "%H:%M", tick_time );
  } else {
    strftime( buffer, sizeof( "00:00" ), "%I:%M", tick_time );
  }

  // Display this time on the TextLayer
  text_layer_set_text( s_time_layer, buffer );

  strftime( date, sizeof( "00 Month" ), "%d %b", tick_time );
  text_layer_set_text( s_date_layer, date );
}

static void tick_handler( struct tm *tick_time, TimeUnits units_changed ) {
  update_time();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Register services
  tick_timer_service_subscribe( MINUTE_UNIT, tick_handler );
  // ... more services

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers( s_main_window, ( WindowHandlers ) {
    .load = main_window_load,
    .unload = main_window_unload
  } );

  // Show the Window on the watch, with animated=true
  window_stack_push( s_main_window, true );
}

static void deinit() {
  // Destroy Window
  window_destroy( s_main_window );
}

int main( void ) {
  init();
  app_event_loop();
  deinit();
}
