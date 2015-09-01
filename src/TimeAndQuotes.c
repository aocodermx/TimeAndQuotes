#include <pebble.h>


// Definitions for Main Window
#define BAR_DATETIME_HEIGTH   35
#define BAR_AUTHOR            20
#define PHRASE_TO_DISPLAY_KEY 1
#define PHRASE_DAY_KEY        2

static Window    *s_main_window;
static TextLayer *s_quote_layer;
static TextLayer *s_quote_author_layer;
static TextLayer *s_time_layer;
static int phrase_day            = 0;
static int phrase_to_display     = 0;
const char * const phrase_list[] = {
  "Si no quieres una respuesta sarcástica, entonces no hagas una pregunta estúpida.", "Friedrich Nietzsche",
  "Porque morir no duele, lo que duele es el olvido.", "Subcomandante Marcos",
  "Tanto si crees que puedes hacerlo, como si no, en los dos casos tienes razón.", "Henry Ford",
  "Quien controla el presente controla el pasado y quien controla el pasado controlara el futuro.", "George Orwell",
  "Lo supremo en el arte de la guerra consiste en someter al enemigo sin darle batalla.", "Sun-Tzu",
  "Hazte amigo del dolor, y nunca te encontraras solo.", "Ken Chlouber",
  "El que se queda haciendo siempre lo mismo, a la larga se queda atras.", "Andres Oppenheimer",
  "Somos el color de la tierra.", "Subcomandante Marcos.",
  "No podemos resolver nuestros problemas con el mismo pensamiento que usamos cuando los creamos", "Albert Einstein",
  NULL
};

static void update_quote  ( bool next_quote );
static void update_time   ( );
static void tick_handler  ( struct tm *, TimeUnits );
static void tap_handler   ( AccelAxisType , int32_t );
static void timer_handler ( void * );


// Definitions for calendar Window
#define BAR_CALENDAR_TOP_HEIGTH 30
#define CALENDAR_DISPLAY_TIME   3000

static Window    *s_calendar_window;
static TextLayer *s_date_layer;
static Layer     *s_calendar_layer;
const char * const weekdays[] = { "D", "L", "M", "M", "J", "V", "S" };

static void calendar_layer_update ( Layer *, GContext *);
static int get_weekday            ( struct tm );



/*
 *  IMPLEMENTATION FOR MAIN WINDOW
 */
static void main_window_load ( Window *window ) {
  Layer *window_layer = window_get_root_layer ( window );
  GRect bounds        = layer_get_bounds      ( window_layer );

  accel_tap_service_subscribe  ( tap_handler );

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

  phrase_to_display = persist_exists ( PHRASE_TO_DISPLAY_KEY ) ? persist_read_int ( PHRASE_TO_DISPLAY_KEY ) : 0;
  phrase_day        = persist_exists ( PHRASE_DAY_KEY ) ? persist_read_int ( PHRASE_DAY_KEY ): 0;

  update_quote ( false );
  update_time();
}

static void update_quote ( bool next_quote ) {
  int count = 0;
  GFont quote_font;

  while ( phrase_list[count] != NULL ) count++;
  if ( phrase_to_display > count - 1 ) phrase_to_display = 0;

  const char * quote  = phrase_list[ phrase_to_display ];
  const char * author = phrase_list[ phrase_to_display + 1 ];

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

  if ( next_quote == true ) phrase_to_display += 2;
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
  phrase_day = persist_exists ( PHRASE_DAY_KEY ) ? persist_read_int ( PHRASE_DAY_KEY ): 0;
  if ( phrase_day != tick_time->tm_mday ) {
    update_quote ( true );
    phrase_day = tick_time->tm_mday;
    persist_write_int ( PHRASE_DAY_KEY, phrase_day );
  }

  update_time ( );
  /*  THIS DOESN'T WORK
  switch ( units_changed ) {
    case DAY_UNIT:
      update_quote ( true );
      break;

    default:
      update_time ( );
      break;
  }
  */
}

static void tap_handler ( AccelAxisType axis, int32_t direction ) {
  Window *topmost = window_stack_get_top_window ( );
  if ( topmost == s_main_window ) {
    window_stack_push ( s_calendar_window, true );
  } else {
    // Handle taps interaction for slide calendar
  }
}

static void main_window_unload ( Window *window ) {
  accel_tap_service_unsubscribe ( );

  persist_write_int ( PHRASE_TO_DISPLAY_KEY, phrase_to_display );
  persist_write_int ( PHRASE_DAY_KEY, phrase_day );

  text_layer_destroy ( s_quote_layer );
  text_layer_destroy ( s_quote_author_layer );
  text_layer_destroy ( s_time_layer );
}




/*
 *  IMPLEMENTATION FOR CALENDAR WINDOW
 */
static void calendar_window_load ( Window *window ) {
  static char date[20];
  time_t temp          = time                  ( NULL );
  Layer *window_layer  = window_get_root_layer ( window );
  GRect bounds         = layer_get_bounds      ( window_layer );
  struct tm *tick_time = localtime             ( &temp );

  strftime ( date, sizeof( date ), "%B %Y", tick_time );
  app_timer_register ( CALENDAR_DISPLAY_TIME, timer_handler, NULL );

  // Create date Calendar Layer
  s_calendar_layer = layer_create ( GRect ( 0, BAR_CALENDAR_TOP_HEIGTH, bounds.size.w, bounds.size.h - BAR_CALENDAR_TOP_HEIGTH ) );
  layer_set_update_proc ( s_calendar_layer, calendar_layer_update );
  layer_add_child ( window_get_root_layer( window ), s_calendar_layer );

  // Create date TextLayer
  s_date_layer = text_layer_create ( GRect ( 0, 0, bounds.size.w, BAR_CALENDAR_TOP_HEIGTH ) );
  text_layer_set_background_color ( s_date_layer, GColorBlack );
  text_layer_set_text_color       ( s_date_layer, GColorClear );
  text_layer_set_text             ( s_date_layer, date );
  text_layer_set_text_alignment   ( s_date_layer, GTextAlignmentCenter );
  text_layer_set_font             ( s_date_layer, fonts_get_system_font( FONT_KEY_GOTHIC_24_BOLD ) );
  layer_add_child ( window_get_root_layer ( window ) , text_layer_get_layer ( s_date_layer ) );
}

static void calendar_layer_update ( Layer *layer, GContext *ctx ) {
  GPoint p1, p2;
  char day_number_string[10];
  GRect frame, bounds = layer_get_bounds ( layer );

  int week_height = bounds.size.h / 7;
  int day_width   = bounds.size.w / 7;

  time_t temp          = time ( NULL );
  struct tm *tick_time = localtime ( &temp );
  int current_month    = tick_time->tm_mon;
  int current_day      = tick_time->tm_mday;

  tick_time->tm_mday = 1;
  while ( get_weekday ( *tick_time ) != 0 ) tick_time->tm_mday--;

  for ( int i = 0; i <= 6; i++ ) {
    p1 = GPoint ( 0, week_height * i );
    p2 = GPoint ( bounds.size.w, week_height * i );

    graphics_context_set_stroke_color ( ctx, GColorBlack );
    graphics_draw_line ( ctx, p1, p2 );

    for ( int j = 0; j<= 6; j++) {
      GFont days_font;
      if ( get_weekday ( *tick_time ) == j ) {
        snprintf ( day_number_string, sizeof ( day_number_string ) , "%d", tick_time->tm_mday );
        days_font = ( current_month == tick_time->tm_mon ) ? fonts_get_system_font( FONT_KEY_GOTHIC_14_BOLD ) : fonts_get_system_font( FONT_KEY_GOTHIC_14 );

        if ( current_day == tick_time->tm_mday && current_month == tick_time->tm_mon ) {
          graphics_context_set_text_color ( ctx, GColorBlack );
          graphics_fill_rect              ( ctx, GRect ( day_width * j, week_height * i, day_width, week_height ), 0, GCornerNone );
          graphics_context_set_text_color ( ctx, GColorClear );
        } else {
          graphics_context_set_text_color ( ctx, GColorBlack );
        }

        graphics_draw_text ( ctx, day_number_string, days_font, GRect ( day_width * j, week_height * i, day_width, week_height ), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL );
        tick_time->tm_mday++;
      }
    }

    if ( i == 6 ) { // Draw days init
      frame = GRect ( 0, week_height * i, bounds.size.w , week_height * i );

      graphics_context_set_text_color ( ctx, GColorBlack );
      graphics_fill_rect              ( ctx, frame, 0, GCornerNone );

      for ( int j = 0; j<= 6; j++) {
        graphics_context_set_text_color ( ctx, GColorClear );
        graphics_draw_text( ctx, weekdays[j], fonts_get_system_font ( FONT_KEY_GOTHIC_14 ), GRect ( day_width * j, week_height * i, day_width, week_height ), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL );
      }
      // To redraw month and year.
      layer_mark_dirty ( text_layer_get_layer ( s_date_layer ) );
    }
  }
}

static int get_weekday ( struct tm time ) {
  char str[30];
  if ( strftime ( str, sizeof( str ), "%d-%m-%Y", &time ) != 0 ) {
    time_t t = mktime ( &time );
    return localtime ( &t )->tm_wday;
  }
  return -1;
}

static void calendar_window_unload ( Window *window ) {
  text_layer_destroy ( s_date_layer );
}

static void timer_handler ( void* data ) {
  window_stack_remove ( s_calendar_window, true );
}



/*
 * APP LIFECYCLE IMPLEMENTATION
 */
static void init( ) {
  s_main_window     = window_create();
  s_calendar_window = window_create();

  APP_LOG ( APP_LOG_LEVEL_INFO, "Set to: %d", MINUTE_UNIT );
  tick_timer_service_subscribe ( MINUTE_UNIT , tick_handler );

  window_set_window_handlers ( s_main_window, ( WindowHandlers ) {
    .load   = main_window_load,
    .unload = main_window_unload
  } );

  window_set_window_handlers ( s_calendar_window, ( WindowHandlers) {
    .load   = calendar_window_load,
    .unload = calendar_window_unload
  } );

  window_stack_push ( s_main_window, true );
}

static void deinit( ) {
  window_destroy ( s_main_window );
  window_destroy ( s_calendar_window );
}


/*
 * MAIN ROUTINE
 */
int main ( void ) {
  init ( );
  app_event_loop ( );
  deinit ( );
}
