#include <pebble.h>
#include "Calendar.h"

// Definitions for calendar Window
static Window    *s_calendar_window;
static TextLayer *s_date_layer;
static Layer     *s_calendar_layer;
const char * const weekdays[] = { "D", "L", "M", "M", "J", "V", "S" };

static void calendar_window_load ( Window * );
static void calendar_window_unload ( Window * );
static void calendar_layer_update ( Layer *, GContext *);
static int get_weekday            ( struct tm );
static void timer_handler ( void * );

/*
 *  IMPLEMENTATION FOR CALENDAR WINDOW
 */
void window_calendar_init() {
 s_calendar_window = window_create();

 window_set_window_handlers ( s_calendar_window, ( WindowHandlers) {
   .load   = calendar_window_load,
   .unload = calendar_window_unload
 } );

 window_stack_push ( s_calendar_window, true );
}

void window_calendar_deinit() {
 window_destroy ( s_calendar_window );
}

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

static void calendar_window_unload ( Window *window ) {
  text_layer_destroy ( s_date_layer );
}

static int get_weekday ( struct tm time ) {
  char str[30];
  if ( strftime ( str, sizeof( str ), "%d-%m-%Y", &time ) != 0 ) {
    time_t t = mktime ( &time );
    return localtime ( &t )->tm_wday;
  }
  return -1;
}

static void timer_handler ( void* data ) {
  window_stack_remove ( s_calendar_window, true );
}