#include <pebble.h>
#include "QuoteWatchface.h"
#include "Calendar.h"

// Comment for supress debug info, recommended to production environment.
// #define DEBUG_PRINT_LOGS

static Window    *s_quotes_window;
static Layer     *s_battery_layer;
static TextLayer *s_quote_layer;
static TextLayer *s_quote_author_layer;
static TextLayer *s_time_layer;

static int  data_background_color;
static int  data_time_24_hours;
static int  data_show_calendar;
static int  data_show_battery;
static int  data_change_quote;
static char data_quote [MAX_DATA_QUOTE ];
static char data_author[MAX_DATA_AUTHOR];
#if defined(PBL_BW)
  static int s_font_handles [] = { RESOURCE_ID_QUOTE_38, RESOURCE_ID_QUOTE_36, RESOURCE_ID_QUOTE_33, RESOURCE_ID_QUOTE_30, RESOURCE_ID_QUOTE_27, RESOURCE_ID_QUOTE_24, RESOURCE_ID_QUOTE_21, RESOURCE_ID_QUOTE_18, RESOURCE_ID_QUOTE_15, };
#else
  static int s_font_handles [] = { RESOURCE_ID_QUOTE_38, RESOURCE_ID_QUOTE_36, RESOURCE_ID_QUOTE_34, RESOURCE_ID_QUOTE_32, RESOURCE_ID_QUOTE_30, RESOURCE_ID_QUOTE_28, RESOURCE_ID_QUOTE_26, RESOURCE_ID_QUOTE_24, RESOURCE_ID_QUOTE_22, RESOURCE_ID_QUOTE_20, RESOURCE_ID_QUOTE_18, RESOURCE_ID_QUOTE_16, RESOURCE_ID_QUOTE_14, };
#endif
static GFont s_font_quote [ sizeof s_font_handles ];

static int  s_battery_percentage;
static bool s_flag_phone_connected;
static bool s_flag_quote_request_pending;

static void main_window_load      ( Window * );
static void main_window_unload    ( Window * );
static void main_window_appear    ( Window * );
static void main_window_disappear ( Window * );
static void update_quote_layer    ( );
static void update_time_layer     ( );
static void update_battery_layer  ( Layer *, GContext * );

static void handler_tick_service    ( struct tm *, TimeUnits );
static void handler_tap_service     ( AccelAxisType , int32_t );
static void handler_conn_service    ( bool );
static void handler_battery_service ( BatteryChargeState );
static void handler_inbox_success   ( DictionaryIterator *, void *                  );
static void handler_inbox_failed    ( AppMessageResult    , void *                  );
static void handler_outbox_success  ( DictionaryIterator *, void *                  );
static void handler_outbox_failed   ( DictionaryIterator *, AppMessageResult, void *);

/*
 *  PUBLIC METHODS FOR INIT AND DEINIT APPLICATION
 */
void window_watchface_init ( ) {
  s_quotes_window = window_create();

  window_set_window_handlers ( s_quotes_window, ( WindowHandlers ) {
    .load      = main_window_load  ,
    .unload    = main_window_unload,
    .appear    = main_window_appear,
    .disappear = main_window_disappear
  } );

  app_message_register_inbox_received ( handler_inbox_success  );
  app_message_register_inbox_dropped  ( handler_inbox_failed   );
  app_message_register_outbox_failed  ( handler_outbox_failed  );
  app_message_register_outbox_sent    ( handler_outbox_success );
  app_message_open ( 250, 250 );

  window_stack_push ( s_quotes_window, true );
}


void window_watchface_deinit() {
  window_destroy ( s_quotes_window );
}


/*
 *  IMPLEMENTATION FOR APPLICATION LIFECYCLE METHODS
 */
static void main_window_load ( Window *window ) {
  memset ( s_font_quote, 0, sizeof ( s_font_quote ) );

  Layer *window_layer    = window_get_root_layer ( window );
  GRect bounds           = layer_get_bounds      ( window_layer );

  s_flag_phone_connected = connection_service_peek_pebble_app_connection ( );
  s_battery_percentage   = ( battery_state_service_peek ( ) ).charge_percent;

  data_background_color  = persist_exists ( KEY_BACKGROUND_COLOR ) ? persist_read_int ( KEY_BACKGROUND_COLOR ) : 0; // Use watch_info_get_color for better selection
  data_time_24_hours     = persist_exists ( KEY_TIME_24_HOURS    ) ? persist_read_int ( KEY_TIME_24_HOURS    ) : 0;
  data_show_calendar     = persist_exists ( KEY_SHOW_CALENDAR    ) ? persist_read_int ( KEY_SHOW_CALENDAR    ) : 1;
  data_change_quote      = persist_exists ( KEY_CHANGE_QUOTE     ) ? persist_read_int ( KEY_CHANGE_QUOTE     ) : 8;
  data_show_battery      = persist_exists ( KEY_SHOW_BATTERY     ) ? persist_read_int ( KEY_SHOW_BATTERY     ) : 0;
  if ( persist_exists ( KEY_QUOTE ) && persist_exists ( KEY_AUTHOR ) ) {
    persist_read_string ( KEY_QUOTE , data_quote , MAX_DATA_QUOTE  );
    persist_read_string ( KEY_AUTHOR, data_author, MAX_DATA_AUTHOR );
  } else {
    strcpy ( data_quote , DEFAULT_QUOTE  );
    strcpy ( data_author, DEFAULT_AUTHOR );
  }

  // Services suscriptions
  tick_timer_service_subscribe    ( MINUTE_UNIT , handler_tick_service );
  battery_state_service_subscribe ( handler_battery_service            );
  connection_service_subscribe    ( ( ConnectionHandlers               ) {
    .pebble_app_connection_handler = handler_conn_service,
    .pebblekit_connection_handler  = NULL
  } );

  #if defined(PBL_COLOR)
    GColor bg_color = GColorFromHEX ( data_background_color );
    window_set_background_color ( window, bg_color );
  #else
    window_set_background_color ( window, GColorBlack );
  #endif

  // Create battery layer for display battery percentage
  s_battery_layer = layer_create ( GRect ( 0, 0, bounds.size.w, BAR_BATTERY_HEIGHT ) );
  layer_set_update_proc ( s_battery_layer, update_battery_layer );
  layer_add_child       ( window_layer   , s_battery_layer      );

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
  layer_add_child ( window_layer, text_layer_get_layer ( s_quote_layer ) );

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
  layer_add_child ( window_layer, text_layer_get_layer ( s_quote_author_layer ) );

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
  layer_add_child ( window_layer, text_layer_get_layer ( s_time_layer ) );

  update_quote_layer ( );
  update_time_layer ( );

  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG ( APP_LOG_LEVEL_INFO, "MAIN_WINDOW_LOAD: heap_bytes_used %d", heap_bytes_used ( ) );
  #endif
}

static void main_window_unload ( Window *window ) {
  layer_destroy      ( s_battery_layer );
  text_layer_destroy ( s_quote_layer );
  text_layer_destroy ( s_quote_author_layer );
  text_layer_destroy ( s_time_layer );

  tick_timer_service_unsubscribe    ( );
  connection_service_unsubscribe    ( );
  battery_state_service_unsubscribe ( );
  accel_tap_service_unsubscribe     ( );

  for ( int i = 0; i < (int) ( sizeof ( s_font_handles ) / sizeof ( s_font_handles[0] ) ); i++ ) {
    if ( s_font_quote [ i ] != 0 ) {
      fonts_unload_custom_font ( s_font_quote [ i ] );
      s_font_quote [ i ] = 0;
    }
  }

  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG ( APP_LOG_LEVEL_INFO, "MAIN_WINDOW_UNLOAD: heap_bytes_used %d", heap_bytes_used ( ) );
  #endif
}

static void main_window_appear ( Window *window ) {
  if ( data_show_calendar == true ) {
    accel_tap_service_subscribe  ( handler_tap_service );
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

static void update_quote_layer ( ) {
  GSize qBox;
  GRect bounds = layer_get_bounds ( window_get_root_layer ( s_quotes_window ) );

  #if defined(PBL_COLOR)
    window_set_background_color ( s_quotes_window, GColorFromHEX ( data_background_color ) );
  #endif

  int quote_height_limit = ( data_show_battery == 1 ) ?
    bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR - BAR_BATTERY_HEIGHT - 5:
    bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR - 5;

  // Unload previous used fonts.
  for ( int i = 0; i < (int) ( sizeof ( s_font_handles ) / sizeof ( s_font_handles[0] ) ); i++ ) {
    if ( s_font_quote [ i ] != 0 ) {
      fonts_unload_custom_font ( s_font_quote [ i ] );
      s_font_quote [ i ] = 0;
      #if defined(DEBUG_PRINT_LOGS)
        APP_LOG ( APP_LOG_LEVEL_INFO, "UPDATE_QUOTE: Unload font %d, heap_bytes_used %d", i, heap_bytes_used ( ) );
      #endif
    }
  }

  text_layer_set_size ( s_quote_layer, GSize ( bounds.size.w, bounds.size.h ) );
  text_layer_set_text ( s_quote_author_layer, data_author);

  for ( int i = 0; i < (int) ( sizeof ( s_font_handles ) / sizeof ( s_font_handles[0] ) ); i++ ) {
    if ( s_font_quote [ i ] == 0 ) {
      s_font_quote [i] = fonts_load_custom_font ( resource_get_handle ( s_font_handles[i] ) );
      #if defined(DEBUG_PRINT_LOGS)
        APP_LOG ( APP_LOG_LEVEL_INFO, "UPDATE_QUOTE: Load font %d, heap_bytes_used %d", i, heap_bytes_used ( ) );
      #endif
    }
    text_layer_set_font ( s_quote_layer, s_font_quote [ i ] );
    text_layer_set_text ( s_quote_layer, data_quote );

    qBox = text_layer_get_content_size ( s_quote_layer );

    if ( qBox.h < quote_height_limit ) {
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
  layer_set_frame ( text_layer_get_layer ( s_quote_layer ), GRect ( bounds.origin.x, bounds.origin.y + ( bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR - qBox.h ) / 2, bounds.size.w, bounds.size.h - BAR_DATETIME_HEIGTH - BAR_AUTHOR ) );
}

static void update_time_layer ( ) {
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_TIME: data_time_24_hours %d, heap_bytes_used: %d", data_time_24_hours, heap_bytes_used ( ) );
  #endif
  static char buffer[] = "00:00 XX.";
  time_t temp          = time      ( NULL );
  struct tm *tick_time = localtime ( &temp );

  // Update connection status from handler_conn_service
  if ( data_time_24_hours == true ) {
    if ( s_flag_phone_connected )
      strftime ( buffer, sizeof( "00 : 00." ), "%H : %M.", tick_time );
    else
      strftime ( buffer, sizeof( "00 : 00" ), "%H : %M", tick_time );
  } else {
    if ( s_flag_phone_connected )
      strftime ( buffer, sizeof( "00:00 XX." ), "%I:%M %p.", tick_time );
    else
      strftime ( buffer, sizeof( "00:00 XX" ), "%I:%M %p", tick_time );
  }

  text_layer_set_text ( s_time_layer, buffer );
}

static void update_battery_layer ( Layer *layer, GContext *ctx ) {
  if ( data_show_battery ) {
    GColor bg_color   = GColorFromHEX ( data_background_color );
    GRect bounds      = layer_get_bounds ( layer );

    //int battery_width = ( bounds.size.w * s_battery_percentage ) / 100 ;
    // GRect rect_bounds = GRect ( ( bounds.size.w - battery_width ) / 2, 0, battery_width, BAR_BATTERY_HEIGHT );

    #if defined(PBL_COLOR)
      graphics_context_set_stroke_color ( ctx, gcolor_legible_over ( bg_color ) );
      graphics_context_set_fill_color   ( ctx, gcolor_legible_over ( bg_color ) );
    #else
      graphics_context_set_stroke_color ( ctx, GColorWhite );
      graphics_context_set_fill_color   ( ctx, GColorWhite );
    #endif

    int dots    = s_battery_percentage / 10;
    int space   = bounds.size.w / 20;
    int start_y = BAR_BATTERY_HEIGHT / 2;
    // int start_x = bounds.size.w / 4 + ( ( 10 - dots ) * space ) / 2 + start_y + 2;
    int start_x = ( bounds.size.w - ( dots * space ) ) / 2 + 2 + BAR_BATTERY_HEIGHT ;

    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_BATTERY_LAYER: Width  : %d", bounds.size.w );
      APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_BATTERY_LAYER: Dots   : %d", dots );
      APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_BATTERY_LAYER: Space  : %d", space );
      APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_BATTERY_LAYER: start_x: %d", start_x );
      APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_BATTERY_LAYER: start_y: %d", start_y );
    #endif

    for ( int i = 0; i<dots; i++ ) {
      graphics_draw_circle ( ctx, GPoint ( start_x + space * i, start_y ), BAR_BATTERY_HEIGHT / 2 );
      graphics_fill_circle ( ctx, GPoint ( start_x + space * i, start_y ), BAR_BATTERY_HEIGHT / 2 );

      #if defined(DEBUG_PRINT_LOGS)
        APP_LOG( APP_LOG_LEVEL_INFO, "UPDATE_BATTERY_LAYER: Draw point x:%d, y:%d, r:%d", start_x + space * i, start_y, BAR_BATTERY_HEIGHT / 2 );
      #endif
    }

    // graphics_draw_rect ( ctx, rect_bounds );
    // graphics_fill_rect ( ctx, rect_bounds, 0, GCornerNone );
  }
}



/*
 *  HANDLER FOR SERVICES SUSCRIPTION
 */
static void handler_tick_service ( struct tm *tick_time, TimeUnits units_changed ) {
  update_time_layer ( );

  if( ( ( units_changed & data_change_quote ) != 0 || s_flag_quote_request_pending == true ) && s_flag_phone_connected == true ) {
    int result = 0;
    int value  = 1;

    DictionaryIterator *request;
    app_message_outbox_begin ( &request );
    dict_write_int ( request, KEY_REQUEST_QUOTE, &value, sizeof(int), false );
    result = app_message_outbox_send ( );

    s_flag_quote_request_pending = ( result == APP_MSG_OK ) ? false : true;
    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG( APP_LOG_LEVEL_INFO, "TICK_HANDLER: New quote request result %d", result );
    #endif
  }
}

static void handler_tap_service ( AccelAxisType axis, int32_t direction ) {
    window_calendar_init();
    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG( APP_LOG_LEVEL_INFO, "TAP_HANDLER: Tap Succed, calling window_calendar" );
    #endif
}

static void handler_conn_service ( bool connected ) {
  s_flag_phone_connected = connected;
  update_time_layer ( ); // Instead update bluetooth connection icon
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG( APP_LOG_LEVEL_INFO, "CONNECTION_HANDLER: Connection status changed %d", connected );
  #endif
}

static void handler_battery_service ( BatteryChargeState status ) {
  s_battery_percentage = status.charge_percent;
  layer_mark_dirty ( s_battery_layer );
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG( APP_LOG_LEVEL_INFO, "BATTERY_HANDLER: Battery status changed %d%%, charging %d, plugged %d", status.charge_percent, status.is_charging, status.is_plugged );
  #endif
}

static void handler_inbox_success(DictionaryIterator *iterator, void *context) {
  Tuple *key_quote  = dict_find ( iterator, KEY_QUOTE );
  Tuple *key_author = dict_find ( iterator, KEY_AUTHOR);

  if ( key_quote && key_author ) {
    strcpy ( data_quote, (char *)key_quote->value->cstring );
    strcpy ( data_author, (char *)key_author->value->cstring );

    persist_write_string ( KEY_QUOTE           , data_quote );
    persist_write_string ( KEY_AUTHOR          , data_author );

    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_QUOTE  %s", data_quote);
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_AUTHOR %s", data_author);
    #endif

    update_quote_layer();
  }

  Tuple *key_background_color = dict_find ( iterator, KEY_BACKGROUND_COLOR );
  Tuple *key_time_24_hours    = dict_find ( iterator, KEY_TIME_24_HOURS    );
  Tuple *key_show_calendar    = dict_find ( iterator, KEY_SHOW_CALENDAR    );
  Tuple *key_change_quote     = dict_find ( iterator, KEY_CHANGE_QUOTE     );
  Tuple *key_show_battery     = dict_find ( iterator, KEY_SHOW_BATTERY     );

  if ( key_background_color && key_time_24_hours && key_show_calendar && key_change_quote ) {
    data_background_color = (int)key_background_color->value->int32;
    data_time_24_hours    = (int)key_time_24_hours->value->uint8;
    data_show_calendar    = (int)key_show_calendar->value->uint8;
    data_change_quote     = (int)key_change_quote->value->uint8;
    data_show_battery     = (int)key_show_battery->value->uint8;

    persist_write_int ( KEY_BACKGROUND_COLOR, data_background_color );
    persist_write_int ( KEY_TIME_24_HOURS   , data_time_24_hours    );
    persist_write_int ( KEY_SHOW_CALENDAR   , data_show_calendar    );
    persist_write_int ( KEY_CHANGE_QUOTE    , data_change_quote     );
    persist_write_int ( KEY_SHOW_BATTERY    , data_show_battery     );

    #if defined(DEBUG_PRINT_LOGS)
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_BACKGROUND_COLOR %d", data_background_color );
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_TIME_24_HOURS    %d", data_time_24_hours    );
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_SHOW_CALENDAR    %d", data_show_calendar    );
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_CHANGE_QUOTE     %d", data_change_quote     );
      APP_LOG(APP_LOG_LEVEL_INFO, "handler_inbox_success: proccessed KEY_SHOW_BATTERY     %d", data_show_battery     );
    #endif

    if ( data_show_calendar == true ) {
      accel_tap_service_subscribe  ( handler_tap_service );
    } else {
      accel_tap_service_unsubscribe ( );
    }

    if ( data_show_battery == true ) {
      battery_state_service_subscribe ( handler_battery_service );
    } else {
      battery_state_service_unsubscribe ( );
      s_battery_percentage = 0;
    }

    #if defined(PBL_COLOR)
      GColor bg_color = GColorFromHEX ( data_background_color );
      text_layer_set_background_color ( s_quote_layer, bg_color );
      text_layer_set_text_color       ( s_quote_layer, gcolor_legible_over ( bg_color ) );
      text_layer_set_background_color ( s_quote_author_layer, bg_color );
      text_layer_set_text_color       ( s_quote_author_layer, gcolor_legible_over ( bg_color ) );
      text_layer_set_background_color ( s_time_layer, gcolor_legible_over ( bg_color ) );
      text_layer_set_text_color       ( s_time_layer, bg_color );
      window_set_background_color     ( s_quotes_window, bg_color );
    #endif

    layer_mark_dirty ( s_battery_layer );
    update_time_layer();
  }
}

static void handler_inbox_failed(AppMessageResult reason, void *context) {
  s_flag_quote_request_pending = true;
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG(APP_LOG_LEVEL_ERROR, "handler_inbox_failed: Message dropped! -> %d", reason);
  #endif
}

static void handler_outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  s_flag_quote_request_pending = true;
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG(APP_LOG_LEVEL_ERROR, "handler_outbox_failed: Outbox send failed! -> %d", reason);
  #endif
}

static void handler_outbox_success(DictionaryIterator *iterator, void *context) {
  s_flag_quote_request_pending = false;
  #if defined(DEBUG_PRINT_LOGS)
    APP_LOG(APP_LOG_LEVEL_INFO, "handler_outbox_success: Outbox send success!");
  #endif
}
