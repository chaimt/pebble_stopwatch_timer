#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "app.h"
#include "stopwatch.h"
#include "timer.h"
#include "lib.h"

Window window_stopwatch;
TextLayer layer_stopwatch;
TextLayer layer_laps[4];
AppTimerHandle stopwatch_handle;
int stopwatch_state;
int stopwatch_start;
int stopwatch_seconds;
int lap_milliseconds[4] = {0};
int pause_offset; 
int internal_milliseconds;
char stopwatch_text[] = "00:00:00:000";
char lap_text[4][13] = {""};

void window_stopwatch_init() {
    window_init(&window_stopwatch, "stopwatch");
    window_set_click_config_provider(&window_stopwatch, (ClickConfigProvider) stopwatch_config_provider);
    WindowHandlers window_handlers = {
	.load = &window_stopwatch_load
    };
    window_set_window_handlers(&window_stopwatch, window_handlers);
    window_stack_push(&window_stopwatch, true);
}

void stopwatch_config_provider(ClickConfig **config, Window *window) {
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) window_timer_init;
    config[BUTTON_ID_UP]->click.handler = (ClickHandler) stopwatch_go;
    config[BUTTON_ID_UP]->long_click.handler = (ClickHandler) stopwatch_reset;
    // have to add a long_click.release_handler or the next short click will not be handled
    config[BUTTON_ID_UP]->long_click.release_handler = (ClickHandler) stopwatch_reset;
    config[BUTTON_ID_UP]->long_click.delay_ms = 1000;
    config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) stopwatch_pause;
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) init_timer;
}

void init_timer(ClickConfig **conig, Window *window) {
    window_timer_init();
}

void window_stopwatch_load(Window *window) {
    Layer *root = window_get_root_layer(window);

    stopwatch_state = STOPWATCH_STATE_IDLE;
    stopwatch_seconds = 0;
    stopwatch_start = 0;
    pause_offset = 0;
    internal_milliseconds = 0;

    text_layer_init(&layer_stopwatch, GRect(5,0,144,30));
    text_layer_set_font(&layer_stopwatch, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layer_stopwatch, GColorBlack);
    text_layer_set_text(&layer_stopwatch, "00:00:00:000");
    layer_add_child(root, &layer_stopwatch.layer);

    for(int i=0; i < 4; i++) {
	text_layer_init(&layer_laps[i], GRect(5,0+(i+1)*30,144,30));
	text_layer_set_font(&layer_laps[i], fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_color(&layer_laps[i], GColorBlack);
        text_layer_set_text(&layer_laps[i], "");
	layer_add_child(root, &layer_laps[i].layer);
    }
}

void display_laps() {
    for(int i=0; i < 4; i++) {
	if( lap_milliseconds[i] > 0 ) {
	    convert_milliseconds_to_text(lap_milliseconds[i], lap_text[i]);
	    text_layer_set_text(&layer_laps[i], lap_text[i]);
	    layer_mark_dirty(&layer_laps[i].layer);
	}
    }
}

void stopwatch_go(ClickRecognizerRef recognizer, Window *window) {
    switch(stopwatch_state) {
	case STOPWATCH_STATE_IDLE:
	    stopwatch_start = get_ticks_now_in_seconds();
	    stopwatch_handle = app_timer_send_event(appCtx, STOPWATCH_TICK, COOKIE_STOPWATCH);
	    stopwatch_state = STOPWATCH_STATE_RUNNING;
	    break;
	case STOPWATCH_STATE_RUNNING:

	    for(int i=3; i > 0; i--) {
		lap_milliseconds[i] = lap_milliseconds[i-1]; 
	    }
	    lap_milliseconds[0] = (stopwatch_seconds + pause_offset) * 1000 + internal_milliseconds;
	    display_laps();
	    break;
	case STOPWATCH_STATE_PAUSED:
	    stopwatch_start = get_ticks_now_in_seconds();
	    stopwatch_handle = app_timer_send_event(appCtx, STOPWATCH_TICK, COOKIE_STOPWATCH);
	    stopwatch_state = STOPWATCH_STATE_RUNNING;
	    break;
    }
}

void stopwatch_pause(ClickRecognizerRef recognizer, Window *window) {
    app_timer_cancel_event(appCtx, stopwatch_handle);
    pause_offset += stopwatch_seconds;
    stopwatch_seconds = 0;
    stopwatch_state = STOPWATCH_STATE_PAUSED;
}

void stopwatch_reset(ClickRecognizerRef recognizer, Window *window) {
    app_timer_cancel_event(appCtx, stopwatch_handle);
    stopwatch_state = STOPWATCH_STATE_IDLE;
    stopwatch_seconds = 0;
    pause_offset = 0;

    strcpy(stopwatch_text,"00:00:00:000");    
    layer_mark_dirty(&layer_stopwatch.layer);

    for(int i=0; i < 4; i++) {	
	strcpy(lap_text[i], "");
	lap_milliseconds[i] = 0;
	layer_mark_dirty(&layer_laps[i].layer);
    }
}

void stopwatch_handle_timer(AppContextRef ctx, AppTimerHandle handle) {

    int seconds_now = get_ticks_now_in_seconds();
    int current_seconds = seconds_now - stopwatch_start;

    //reset internal milliseconds on tick to reset drift    
    if( stopwatch_seconds != current_seconds ) {	
	stopwatch_seconds = current_seconds;
	internal_milliseconds = 0;
    }
    else {
	internal_milliseconds += STOPWATCH_TICK;
    }
    
    int total_milliseconds = (stopwatch_seconds + pause_offset)*1000 + internal_milliseconds;
    convert_milliseconds_to_text(total_milliseconds, stopwatch_text);
    
    text_layer_set_text(&layer_stopwatch, stopwatch_text);
    layer_mark_dirty(&layer_stopwatch.layer);
    stopwatch_handle = app_timer_send_event(appCtx, STOPWATCH_TICK, COOKIE_STOPWATCH);
}

void convert_milliseconds_to_text(int ticks, char *text) {

    int milliseconds = ticks % 1000;
    int seconds = ticks / 1000 % 60;
    int minutes = ticks  / 60 / 1000 % 60;
    int hours = ticks  / 60 / 60 / 1000;

    strcpy(text, itoa(hours,2));
    strcat(text, ":");
    strcat(text, itoa(minutes,2));
    strcat(text, ":");
    strcat(text, itoa(seconds,2));
    strcat(text, ":");
    strcat(text, itoa(milliseconds,3));
}


