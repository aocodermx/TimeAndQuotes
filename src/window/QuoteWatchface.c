#include <pebble.h>
#include "QuoteWatchface.h"
#include "Calendar.h"

// Comment for supress debug info, recommended to production environment.
#define DEBUG_PRINT_LOGS

static Window    *s_quotes_window;
static TextLayer *s_quote_layer;
static TextLayer *s_quote_author_layer;
static TextLayer *s_time_layer;
#if defined(PBL_BW)
  static int font_handles [] = {
    RESOURCE_ID_QUOTE_37,
    RESOURCE_ID_QUOTE_34,
    RESOURCE_ID_QUOTE_31,
    RESOURCE_ID_QUOTE_28,
    RESOURCE_ID_QUOTE_25,
    RESOURCE_ID_QUOTE_22,
    RESOURCE_ID_QUOTE_19,
    RESOURCE_ID_QUOTE_16,
    RESOURCE_ID_QUOTE_13,
    RESOURCE_ID_QUOTE_10,
  };
#else
  static int font_handles [] = {
    RESOURCE_ID_QUOTE_38,
    RESOURCE_ID_QUOTE_36,
    RESOURCE_ID_QUOTE_34,
    RESOURCE_ID_QUOTE_32,
    RESOURCE_ID_QUOTE_30,
    RESOURCE_ID_QUOTE_28,
    RESOURCE_ID_QUOTE_26,
    RESOURCE_ID_QUOTE_24,
    RESOURCE_ID_QUOTE_22,
    RESOURCE_ID_QUOTE_20,
    RESOURCE_ID_QUOTE_18,
    RESOURCE_ID_QUOTE_16,
    RESOURCE_ID_QUOTE_14,
    RESOURCE_ID_QUOTE_12,
    RESOURCE_ID_QUOTE_10,
  };
#endif
static GFont s_font_quote [ sizeof font_handles ];

static int data_background_color;
static int data_time_24_hours;
static int data_show_calendar;
static int data_change_quote;
static char data_quote [MAX_DATA_QUOTE ];
static char data_author[MAX_DATA_AUTHOR];


static void main_window_load      ( Window * );
static void main_window_unload    ( Window * );
static void main_window_appear    ( Window * );
static void main_window_disappear ( Window * );
static void update_quote  ( );
static void update_time   ( );
static void tick_handler  ( struct tm *, TimeUnits );
static void tap_handler   ( AccelAxisType , int32_t );

static void inbox_received_callback(DictionaryIterator *, void *);
static void inbox_dropped_callback(AppMessageResult, void *);
static void outbox_sent_callback(DictionaryIterator *, void *);
static void outbox_failed_callback(DictionaryIterator *, AppMessageResult, void *);

/*
 *  IMPLEMENTATION FOR QUOTEWATCHFACE WINDOW
 */


void window_watchface_init ( ) {
  s_quotes_window = window_create();

  tick_timer_service_subscribe ( MINUTE_UNIT , tick_handler );
  window_set_window_handlers ( s_quotes_window, ( WindowHandlers ) {
    .load   = main_window_load,
    .unload = main_window_unload,
    .appear = main_window_appear,
    .disappear = main_window_disappear
  } );

  app_message_register_inbox_received ( inbox_received_callback );
  app_message_register_inbox_dropped ( inbox_dropped_callback );
  app_message_register_outbox_failed ( outbox_failed_callback );
  app_message_register_outbox_sent ( outbox_sent_callback );
  app_message_open ( 500, 5000 );

  window_stack_push ( s_quotes_window, true );
}


void window_watchface_deinit() {
  window_destroy ( s_quotes_window );
}


static void main_window_load ( Window *window ) {
  Layer *window_layer = window_get_root_layer ( window );
  GRect bounds        = layer_get_bounds      ( window_layer );

  memset ( s_font_quote, 0, sizeof ( s_font_quote ) );

  // Load configuration data
  data_background_color = persist_exists ( KEY_BACKGROUND_COLOR ) ? persist_read_int ( KEY_BACKGROUND_COLOR ) : 0; // Use watch_info_get_color for better selection
  data_time_24_hours    = persist_exists ( KEY_TIME_24_HOURS    ) ? persist_read_int ( KEY_TIME_24_HOURS    ) : 1;
  data_show_calendar    = persist_exists ( KEY_SHOW_CALENDAR    ) ? persist_read_int ( KEY_SHOW_CALENDAR    ) : 1;
  data_change_quote     = persist_exists ( KEY_CHANGE_QUOTE     ) ? persist_read_int ( KEY_CHANGE_QUOTE     ) : 8;
  if ( persist_exists ( KEY_QUOTE ) && persist_exists ( KEY_AUTHOR ) ) {
    persist_read_string ( KEY_QUOTE , data_quote , MAX_DATA_QUOTE  );
    persist_read_string ( KEY_AUTHOR, data_author, MAX_DATA_AUTHOR );
  } else {
    strcpy ( data_quote , DEFAULT_QUOTE  );
    strcpy ( data_author, DEFAULT_AUTHOR );
  }

  #if defined(PBL_COLOR)
    GColor bg_color = GColorFromHEX ( data_background_color );
    window_set_background_color ( window, bg_color );
  #else
    window_set_background_color ( window, GColorBlack );
  #endif

  // Create quote TextLayer and add it to Window hierarchy
  s_quote_layer = text_layer_create ( GRect ( 0, 0, bounds.size.w, bounds.size.h ) );
  #if defined(PBL_COLOR)
    text_layer_set_background_color ( s_quote_layer, bg_color );
    text_layer_set_text_color       ( s_quote_layer, gcolor_legible_over ( bg_color ) );
  #else
    text_layer_set_background_color ( s_quote_layer, GColorBlack );
    text_layer_set_text_color       ( s_quote_layer, GColorWhite );
  #endif
  text_layer_set_text_alignment   ( s_quote_layer, GTextAlignmentCenter );
  layer_add_child ( window_get_root_layer ( window ), text_layer_get_layer ( s_quote_layer ) );

  // Create quote author TextLayer and add it to Window hierarchy
  s_quote_author_layer = text_layer_create ( GRect ( 0, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR, bounds.size.w, BAR_AUTHOR ) );
  #if defined(PBL_COLOR)
    text_layer_set_background_color ( s_quote_author_layer, bg_color );
    text_layer_set_text_color       ( s_quote_author_layer, gcolor_legible_over ( bg_color ) );
  #else
    text_layer_set_background_color ( s_quote_author_layer, GColorBlack );
    text_layer_set_text_color       ( s_quote_author_layer, GColorWhite );
  #endif
  text_layer_set_overflow_mode    ( s_quote_author_layer, GTextOverflowModeTrailingEllipsis );
  text_layer_set_text_alignment   ( s_quote_author_layer, GTextAlignmentCenter );
  text_layer_set_font             ( s_quote_author_layer, fonts_get_system_font ( FONT_KEY_GOTHIC_14 ) );
  layer_add_child ( window_get_root_layer ( window ), text_layer_get_layer ( s_quote_author_layer ) );

  // Create time TextLayer and add it to Windows hierarchy
  s_time_layer = text_layer_create ( GRect ( 0, bounds.size.h - BAR_DATETIME_HEIGTH, bounds.size.w, BAR_DATETIME_HEIGTH ) );
  #if defined(PBL_COLOR)
    text_layer_set_background_color ( s_time_layer, gcolor_legible_over ( bg_color ) );
    text_layer_set_text_color       ( s_time_layer, bg_color );
  #else
    text_layer_set_background_color ( s_time_layer, GColorWhite );
    text_layer_set_text_color       ( s_time_layer, GColorBlack );
  #endif
  text_layer_set_text_alignment   ( s_time_layer, GTextAlignmentCenter );
  text_layer_set_font             ( s_time_layer, fonts_get_system_font( FONT_KEY_DROID_SERIF_28_BOLD ) );
  layer_add_child ( window_get_root_layer ( window ), text_layer_get_layer ( s_time_layer ) );

  update_quote ( );
  update_time ( );

  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG ( APP_LOG_LEVEL_INFO, "MAIN_WINDOW_LOAD: heap_bytes_used %d", heap_bytes_used ( ) );
  #endif
}

static void main_window_unload ( Window *window ) {
  text_layer_destroy ( s_quote_layer );
  text_layer_destroy ( s_quote_author_layer );
  text_layer_destroy ( s_time_layer );

  persist_write_string ( KEY_QUOTE           , data_quote );
  persist_write_string ( KEY_AUTHOR          , data_author );
  persist_write_int    ( KEY_BACKGROUND_COLOR, data_background_color );
  persist_write_int    ( KEY_TIME_24_HOURS   , data_time_24_hours );
  persist_write_int    ( KEY_SHOW_CALENDAR   , data_show_calendar );
  persist_write_int    ( KEY_CHANGE_QUOTE    , data_change_quote );

  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG ( APP_LOG_LEVEL_INFO, "MAIN_WINDOW_UNLOAD: heap_bytes_used %d", heap_bytes_used ( ) );
  #endif
}

static void main_window_appear ( Window *window ) {
  if ( data_show_calendar == true ) {
    accel_tap_service_subscribe  ( tap_handler );
  }
  // To avoid UI to become dirty
  layer_mark_dirty ( window_get_root_layer ( window ) );

  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG ( APP_LOG_LEVEL_INFO, "MAIN_WINDOW_APPEAR: heap_bytes_used %d", heap_bytes_used ( ) );
  #endif
}

static void main_window_disappear ( Window *window ){
  if ( data_show_calendar == true ) {
    accel_tap_service_unsubscribe ( );
  }

  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG ( APP_LOG_LEVEL_INFO, "MAIN_WINDOW_DISAPPEAR: heap_bytes_used %d", heap_bytes_used ( ) );
  #endif
}

static void update_quote ( ) {
  GSize qBox;
  GRect bounds = layer_get_bounds ( window_get_root_layer ( s_quotes_window ) );

  #if defined(PBL_COLOR)
    window_set_background_color ( s_quotes_window, GColorFromHEX ( data_background_color ) );
  #endif

  text_layer_set_size ( s_quote_layer, GSize ( bounds.size.w, bounds.size.h ) );
  text_layer_set_text ( s_quote_author_layer, data_author);

  for ( int i = 0; i < (int) ( sizeof ( font_handles ) / sizeof ( font_handles[0] ) ); i++ ) {
    if ( s_font_quote [ i ] == 0 ) {
      s_font_quote [i] = fonts_load_custom_font ( resource_get_handle ( font_handles[i] ) );
      #if defined(DEBUG_PRINT_LOGS)
        APP_LOG ( APP_LOG_LEVEL_INFO, "UPDATE_QUOTE: Load font %d, heap_bytes_used %d", i, heap_bytes_used ( ) );
      #endif
    }
    text_layer_set_font ( s_quote_layer, s_font_quote [ i ] );
    text_layer_set_text ( s_quote_layer, data_quote );

    qBox = text_layer_get_content_size ( s_quote_layer );

    if ( qBox.h < bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR ) {
      #if defined(DEBUG_PRINT_LOGS)
        APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_QUOTE: Font position: %d, Quote height: %d, Quote max: %d, heap_bytes_used: %d", i, qBox.h, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR, heap_bytes_used ( ) );
      #endif
      break;
    }

    if ( i > 0 ) {
      fonts_unload_custom_font ( s_font_quote [ i - 1 ] );
      s_font_quote [ i - 1 ] = 0;
      #if defined(DEBUG_PRINT_LOGS)
        APP_LOG ( APP_LOG_LEVEL_INFO, "UPDATE_QUOTE: Unload font %d, heap_bytes_used %d", i - 1, heap_bytes_used ( ) );
      #endif
    }
  }

  // Align vertically s_quote_layer
  layer_set_frame ( text_layer_get_layer ( s_quote_layer ), GRect ( bounds.origin.x, bounds.origin.y + ( bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR - qBox.h ) / 2, bounds.size.w, qBox.h ) );
  text_layer_set_size ( s_quote_layer, GSize ( bounds.size.w, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR ) );
}

static void update_time ( ) {
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_TIME: data_time_24_hours %d, heap_bytes_used: %d", data_time_24_hours, heap_bytes_used ( ) );
  #endif
  static char buffer[] = "00:00 XX";
  time_t temp          = time      ( NULL );
  struct tm *tick_time = localtime ( &temp );

  if ( data_time_24_hours == true )
    strftime ( buffer, sizeof( "00 : 00" ), "%H : %M", tick_time );
  else
    strftime ( buffer, sizeof( "00:00 XX" ), "%I:%M %p", tick_time );

  text_layer_set_text ( s_time_layer, buffer );
}

static void tick_handler ( struct tm *tick_time, TimeUnits units_changed ) {
  update_time ( );

  if( ( units_changed & data_change_quote ) != 0 ) {
    int value  = 1;

    DictionaryIterator *request;
    app_message_outbox_begin ( &request );
    dict_write_int ( request, KEY_REQUEST_QUOTE, &value, sizeof(int), false );
    #if defined(DEBUG_PRINT_LOGS)
      int result = app_message_outbox_send ( );
      if ( result == APP_MSG_OK ) {
        APP_LOG( APP_LOG_LEVEL_INFO, "TICK_HANDLER: New quote requested" );
      } else {
        APP_LOG( APP_LOG_LEVEL_INFO, "TICK_HANDLER: New quote request failed %d", result );
      }
    #else
      app_message_outbox_send ( );
    #endif
  }
}

static void tap_handler ( AccelAxisType axis, int32_t direction ) {
    window_calendar_init();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  Tuple *key_quote  = dict_find ( iterator, KEY_QUOTE );
  Tuple *key_author = dict_find ( iterator, KEY_AUTHOR);

  if ( key_quote && key_author ) {
    strcpy ( data_quote, (char *)key_quote->value->cstring );
    strcpy ( data_author, (char *)key_author->value->cstring );

    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG(APP_LOG_LEVEL_INFO, "INBOX_RECEIVED_CALLBACK: proccessed KEY_QUOTE  %s", data_quote);
      APP_LOG(APP_LOG_LEVEL_INFO, "INBOX_RECEIVED_CALLBACK: proccessed KEY_AUTHOR %s", data_author);
    #endif

    update_quote();
  }

  Tuple *key_background_color = dict_find ( iterator, KEY_BACKGROUND_COLOR );
  Tuple *key_time_24_hours    = dict_find ( iterator, KEY_TIME_24_HOURS );
  Tuple *key_show_calendar    = dict_find ( iterator, KEY_SHOW_CALENDAR );
  Tuple *key_change_quote     = dict_find ( iterator, KEY_CHANGE_QUOTE );

  if ( key_background_color && key_time_24_hours && key_show_calendar && key_change_quote ) {
    data_background_color = (int)key_background_color->value->int32;
    data_time_24_hours    = (int)key_time_24_hours->value->uint8;
    data_show_calendar    = (int)key_show_calendar->value->uint8;
    data_change_quote     = (int)key_change_quote->value->uint8;

    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG(APP_LOG_LEVEL_INFO, "INBOX_RECEIVED_CALLBACK: proccessed KEY_BACKGROUND_COLOR %d", data_background_color );
      APP_LOG(APP_LOG_LEVEL_INFO, "INBOX_RECEIVED_CALLBACK: proccessed KEY_TIME_24_HOURS    %d", data_time_24_hours );
      APP_LOG(APP_LOG_LEVEL_INFO, "INBOX_RECEIVED_CALLBACK: proccessed KEY_SHOW_CALENDAR    %d", data_show_calendar );
      APP_LOG(APP_LOG_LEVEL_INFO, "INBOX_RECEIVED_CALLBACK: proccessed KEY_CHANGE_QUOTE     %d", data_change_quote );
    #endif

    if ( data_show_calendar == true ) {
      accel_tap_service_subscribe  ( tap_handler );
    } else {
      accel_tap_service_unsubscribe ( );
    }

    #if defined(PBL_COLOR)
      GColor bg_color = GColorFromHEX ( data_background_color );
      text_layer_set_background_color ( s_quote_layer, bg_color );
      text_layer_set_text_color       ( s_quote_layer, gcolor_legible_over ( bg_color ) );
      text_layer_set_background_color ( s_quote_author_layer, bg_color );
      text_layer_set_text_color       ( s_quote_author_layer, gcolor_legible_over ( bg_color ) );
      text_layer_set_background_color ( s_time_layer, gcolor_legible_over ( bg_color ) );
      text_layer_set_text_color       ( s_time_layer, bg_color );
    #endif

    update_time();
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG(APP_LOG_LEVEL_ERROR, "INBOX_DROPPED_CALLBACK: Message dropped! -> %d", reason);
  #endif
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG(APP_LOG_LEVEL_ERROR, "OUTBOX_FAILED_CALLBACK: Outbox send failed! -> %d", reason);
  #endif
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG(APP_LOG_LEVEL_INFO, "OUTBOX_SENT_CALLBACK: Outbox send success!");
  #endif
}
