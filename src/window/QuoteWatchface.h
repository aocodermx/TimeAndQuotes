
#ifndef WINDOW_INCLUDE_QUOTEWATCHFACE
#define WINDOW_INCLUDE_QUOTEWATCHFACE

// Definitions for Main Window
#define BAR_DATETIME_HEIGTH   35
#define BAR_AUTHOR            20
#define PHRASE_TO_DISPLAY_KEY 1
#define PHRASE_DAY_KEY        2
#define MAX_DATA_QUOTE        300
#define MAX_DATA_AUTHOR       60

// Configuration defines
#define KEY_BACKGROUND_COLOR 1
#define KEY_TIME_24_HOURS    2
#define KEY_SHOW_CALENDAR    3
#define KEY_CHANGE_QUOTE     4
#define KEY_REQUEST_QUOTE    5
#define KEY_QUOTE            6
#define KEY_AUTHOR           7

void window_watchface_init  ();
void window_watchface_deinit();

#endif
