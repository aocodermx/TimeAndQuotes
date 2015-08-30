#include <pebble.h>
#include <time.h>

#define PHRASE_TO_DISPLAY_KEY 1

#define BAR_DATETIME_HEIGTH 35
#define BAR_AUTHOR 20

#define BAR_CALENDAR_TOP_HEIGTH 30
#define CALENDAR_DISPLAY_TIME 4500

static Window    *s_main_window;
static Window    *s_calendar_window;

static TextLayer *s_quote_layer;
static TextLayer *s_quote_author_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static Layer     *s_calendar_layer;

static int phrase_to_display = 0;

const char * const weekdays[] = { "D", "L", "M", "M", "J", "V", "S" };

const char * const phrase_list[] = {
  "Porque morir no duele, lo que duele es el olvido.", "Subcomandante Marcos",
  "Somos el color de la tierra.", "Subcomandante Marcos.",
  "Tanto si crees que puedes hacerlo, como si no, en los dos casos tienes razón.", "Henry Ford",
  "Quien controla el presente controla el pasado y quien controla el pasado controlara el futuro.", "George Orwell",
  "Lo supremo en el arte de la guerra consiste en someter al enemigo sin darle batalla.", "Sun-Tzu",
  NULL
};

static void update_quote( bool next_quote );
static void update_time();
static void tick_handler( struct tm *, TimeUnits );
static void tap_handler( AccelAxisType , int32_t );
static void timer_handler( void * );
static void calendar_layer_update_callback( Layer *, GContext *);
int get_weekday( struct tm );

static void main_window_load( Window *window ) {
  // Get the root layer and their bounds
  Layer *window_layer = window_get_root_layer( window );
  GRect bounds = layer_get_bounds( window_layer );

  // Create quote TextLayer and add it to Window hierarchy
  s_quote_layer = text_layer_create( GRect( 0, 0, bounds.size.w, bounds.size.h - BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color( s_quote_layer, GColorBlack );
  text_layer_set_text_color( s_quote_layer, GColorClear );
  text_layer_set_text_alignment( s_quote_layer, GTextAlignmentLeft );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( s_quote_layer ) );

  // Create quote author TextLayer and add it to Window hierarchy
  s_quote_author_layer = text_layer_create( GRect( 0, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR, bounds.size.w, BAR_AUTHOR ) );
  text_layer_set_background_color( s_quote_author_layer, GColorBlack );
  text_layer_set_text_color( s_quote_author_layer, GColorClear );
  text_layer_set_font( s_quote_author_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
  text_layer_set_text_alignment( s_quote_author_layer, GTextAlignmentRight );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( s_quote_author_layer ) );

  // Create time TextLayer and add it to Windows hierarchy
  s_time_layer = text_layer_create( GRect( 0, bounds.size.h - BAR_DATETIME_HEIGTH, bounds.size.w, BAR_DATETIME_HEIGTH ) );
  text_layer_set_background_color( s_time_layer, GColorClear );
  text_layer_set_text_color( s_time_layer, GColorBlack );
  // text_layer_set_text( s_time_layer, "00:00" );
  text_layer_set_font( s_time_layer, fonts_get_system_font( FONT_KEY_DROID_SERIF_28_BOLD ) );
  text_layer_set_text_alignment( s_time_layer, GTextAlignmentCenter );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( s_time_layer ) );

  phrase_to_display = persist_exists( PHRASE_TO_DISPLAY_KEY ) ? persist_read_int( PHRASE_TO_DISPLAY_KEY ) : 0;

  update_quote( false );
  update_time();
}

static void update_quote( bool next_quote ) {
  int count = 0;
  while( phrase_list[count] != NULL ) count++;

  if ( phrase_to_display > count - 1 ) phrase_to_display = 0;

  const char * quote  = phrase_list[ phrase_to_display ];
  const char * author = phrase_list[ phrase_to_display + 1 ];

  // Count quote lenght
  int quote_len = strlen( quote );
  GFont quote_font;

  // Four levels of font sizes, select the right one
       if ( quote_len < 20 )                    quote_font = fonts_get_system_font( FONT_KEY_BITHAM_30_BLACK );
  else if ( quote_len > 20 && quote_len < 50 )  quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_28 );
  else if ( quote_len > 50 && quote_len < 60 )  quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_24 );
  else if ( quote_len > 60 && quote_len < 120 ) quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_18 );
  else      quote_font = fonts_get_system_font( FONT_KEY_GOTHIC_14 );
  text_layer_set_font( s_quote_layer, quote_font );

  // Print the quote and author
  text_layer_set_text( s_quote_layer, quote );
  text_layer_set_text( s_quote_author_layer, author);

  if ( next_quote == true )
    phrase_to_display += 2;
}

static void update_time() {
  // Get a tm structure
  time_t temp = time( NULL );
  struct tm *tick_time = localtime( &temp );

  // Create a long-lived buffer
  static char buffer[] = "00:00 XX";

  // Write the current hours and minutes into the buffer
  if( clock_is_24h_style() == true ) {
    strftime( buffer, sizeof( "00:00 XX" ), "%H:%M", tick_time );
  } else {
    strftime( buffer, sizeof( "00:00 XX" ), "%I:%M %p", tick_time );
  }

  // Display this time on the TextLayer
  text_layer_set_text( s_time_layer, buffer );
}

static void tick_handler( struct tm *tick_time, TimeUnits units_changed ) {
  switch ( units_changed ) {
    case DAY_UNIT:
      update_quote( true );
      break;
    case MONTH_UNIT:
      // update_calendar();
      break;
    default:
      update_time();
  }
}

static void tap_handler( AccelAxisType axis, int32_t direction ) {
  // Load calendar window
  Window *topmost = window_stack_get_top_window();
  if ( topmost == s_main_window ) {
    window_stack_push( s_calendar_window, true );
  } else {
    // Handle taps interaction for slide calendar
  }
}

static void main_window_unload( Window *window ) {
  // Destroy TextLayers

  persist_write_int( PHRASE_TO_DISPLAY_KEY, phrase_to_display );

  text_layer_destroy( s_quote_layer );
  text_layer_destroy( s_quote_author_layer );
  text_layer_destroy( s_time_layer );
}





static void calendar_window_load( Window *window ) {
  Layer *window_layer = window_get_root_layer( window );
  GRect bounds = layer_get_bounds( window_layer );

  time_t temp = time( NULL );
  struct tm *tick_time = localtime( &temp );

  static char date[] = "00 Month Name Year";
  strftime( date, sizeof( "00 Month Name" ), "%B %Y", tick_time );

  // Registry timer to close calendar window
  app_timer_register( CALENDAR_DISPLAY_TIME, timer_handler, NULL );

  s_calendar_layer = layer_create( GRect( 0, BAR_CALENDAR_TOP_HEIGTH, bounds.size.w, bounds.size.h - BAR_CALENDAR_TOP_HEIGTH ) );
  layer_set_update_proc( s_calendar_layer, calendar_layer_update_callback );
  layer_add_child( window_get_root_layer( window ), s_calendar_layer );

  // Create date TextLayer
  s_date_layer = text_layer_create( GRect( 0, 0, bounds.size.w, BAR_CALENDAR_TOP_HEIGTH ) );
  text_layer_set_background_color( s_date_layer, GColorBlack );
  text_layer_set_text_color( s_date_layer, GColorClear );
  text_layer_set_text( s_date_layer, date );
  text_layer_set_font( s_date_layer, fonts_get_system_font( FONT_KEY_GOTHIC_24_BOLD ) );
  text_layer_set_text_alignment( s_date_layer, GTextAlignmentCenter );
  layer_add_child( window_get_root_layer( window ) , text_layer_get_layer( s_date_layer ) );
}

static void calendar_layer_update_callback(Layer *layer, GContext *ctx) {
  GPoint p1, p2;
  GRect frame, bounds = layer_get_bounds(layer);

  int week_height = bounds.size.h / 7;
  int day_width   = bounds.size.w / 7;

  time_t temp = time( NULL );
  struct tm *tick_time = localtime( &temp );
  char day_number_string[10];
  int current_month = tick_time->tm_mon;
  int current_day = tick_time->tm_mday;

  tick_time->tm_mday = 1;
  while ( get_weekday( *tick_time ) != 0 ) tick_time->tm_mday --;

  for ( int i = 0; i <= 6; i++ ) {
    p1 = GPoint( 0, week_height * i );
    p2 = GPoint( bounds.size.w, week_height * i );
    graphics_context_set_stroke_color( ctx, GColorBlack );
    graphics_draw_line( ctx, p1, p2 );

    for ( int j = 0; j<= 6; j++) {
      GFont days_font;
      if( get_weekday( *tick_time ) == j ) {
        snprintf( day_number_string, sizeof( day_number_string ) , "%d", tick_time->tm_mday );
        // APP_LOG( APP_LOG_LEVEL_INFO, "Real Week Day: %d, Week Day: %d", get_weekday( *tick_time ), j );
        if( current_day == tick_time->tm_mday && current_month == tick_time->tm_mon ) {
          graphics_context_set_text_color( ctx, GColorBlack );
          graphics_fill_rect( ctx, GRect( day_width * j, week_height * i, day_width, week_height ), 0, GCornerNone );
          graphics_context_set_text_color( ctx, GColorClear );
        } else {
          graphics_context_set_text_color( ctx, GColorBlack );
        }
        days_font = ( current_month == tick_time->tm_mon ) ? fonts_get_system_font( FONT_KEY_GOTHIC_14_BOLD ) : fonts_get_system_font( FONT_KEY_GOTHIC_14 );
        graphics_draw_text( ctx, day_number_string, days_font, GRect( day_width * j, week_height * i, day_width, week_height ), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL );
        tick_time->tm_mday ++;
      }
    }

    if ( i == 6 ) { // Draw days init
      frame = GRect( 0, week_height * i, bounds.size.w , week_height * i );
      graphics_context_set_text_color( ctx, GColorBlack );
      graphics_fill_rect( ctx, frame, 0, GCornerNone );

      for ( int j = 0; j<= 6; j++) {
        graphics_context_set_text_color(ctx, GColorClear);
        graphics_draw_text( ctx, weekdays[j],
          fonts_get_system_font( FONT_KEY_GOTHIC_14 ),
          GRect( day_width * j, week_height * i, day_width, week_height ),
          GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL );
      }
    }
  }
}

int get_weekday(struct tm time) {
  char str[30];
  if ( strftime( str, sizeof( str ), "%d-%m-%Y", &time ) != 0 ) {
    time_t t = mktime( &time );
    return localtime( &t )->tm_wday; // Sunday=0, Monday=1, etc.
  }
  return -1;
}

static void calendar_window_unload( Window *window) {
  text_layer_destroy( s_date_layer );
}

static void timer_handler( void* data ) {
  window_stack_remove( s_calendar_window, true );
}




static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Create calendar window
  s_calendar_window = window_create();

  // Register services
  tick_timer_service_subscribe( MINUTE_UNIT | DAY_UNIT | MONTH_UNIT, tick_handler );
  accel_tap_service_subscribe( tap_handler );
  // ... more services

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers( s_main_window, ( WindowHandlers ) {
    .load = main_window_load,
    .unload = main_window_unload
  } );

  window_set_window_handlers( s_calendar_window, ( WindowHandlers) {
    .load = calendar_window_load,
    .unload = calendar_window_unload
  } );

  // Show the Window on the watch, with animated=true
  // window_stack_push( s_main_window, true );
  window_stack_push( s_main_window, true );
}

static void deinit() {
  // Destroy Window
  window_destroy( s_main_window );
  window_destroy( s_calendar_window );
}

int main( void ) {
  init();
  app_event_loop();
  deinit();
}
