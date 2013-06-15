#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "app.h"
#include "timer.h"
#include "lib.h"

AppContextRef appCtx;
Window window_timer;
TextLayer layer_timer_minute;
TextLayer layer_timer_second;
TextLayer layer_timer_colon;
AppTimerHandle timer_handle;
uint32_t config_mode;
int timer_state;
int timer_start;
int pause_offset;
int timer_elapsed;
int timer_value;
char timer_text_minute[3];
char timer_text_second[3];

void window_timer_init() {
    window_init(&window_timer, "timer");
    window_set_click_config_provider(&window_timer, (ClickConfigProvider) timer_config_provider);
    WindowHandlers window_handlers = {
	.load = &window_timer_load
    };
    window_set_window_handlers(&window_timer, window_handlers);
    window_stack_push(&window_timer, true);
}

void window_timer_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    
    config_mode = TIMER_CONFIG_NONE;
    timer_state = TIMER_STATE_IDLE;
    timer_value = 30;
    timer_elapsed = 0;
    pause_offset = 0;

    text_layer_init(&layer_timer_minute, GRect(6,43,60,62));
    text_layer_set_font(&layer_timer_minute, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_color(&layer_timer_minute, GColorBlack);
    layer_add_child(root, &layer_timer_minute.layer);

    text_layer_init(&layer_timer_colon, GRect(67,40,15,50));
    text_layer_set_font(&layer_timer_colon, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_color(&layer_timer_colon, GColorBlack);
    text_layer_set_text(&layer_timer_colon, ":");
    layer_add_child(root, &layer_timer_colon.layer);

    text_layer_init(&layer_timer_second, GRect(82,43,60,62));
    text_layer_set_font(&layer_timer_second, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_color(&layer_timer_second, GColorBlack);
    layer_add_child(root, &layer_timer_second.layer);

    convert_ticks_to_text(timer_value, timer_text_minute, timer_text_second);
    update_display();
}

void timer_config_provider(ClickConfig **config, Window *window) {
    config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_click;
    config[BUTTON_ID_UP]->click.repeat_interval_ms = 30;
    config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_click;
    config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 30;
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) timer_config;
    config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) timer_config_stop;
    config[BUTTON_ID_SELECT]->long_click.release_handler = (ClickHandler) timer_config_stop;
    config[BUTTON_ID_SELECT]->long_click.delay_ms = 1000;
}

void timer_config(ClickRecognizerRef recognizer, Window *window) {

    if( timer_state == TIMER_STATE_IDLE ) {

	switch( config_mode ) {
	    case TIMER_CONFIG_NONE:	    
		config_mode = TIMER_CONFIG_MINUTE;
		minute_config_view();
		break;
	    case TIMER_CONFIG_MINUTE:
		config_mode = TIMER_CONFIG_SECOND;
		second_config_view();
		break;
	    case TIMER_CONFIG_SECOND:
		config_mode = TIMER_CONFIG_NONE;
		normal_view();
		break;
	}

	update_display();
    }
}

void timer_config_stop(ClickRecognizerRef recognizer, Window *window) {
    config_mode = TIMER_CONFIG_NONE;
    update_display();
}

void normal_view() {
    text_layer_set_text_color(&layer_timer_minute, GColorBlack);
    text_layer_set_background_color(&layer_timer_minute, GColorWhite);
    text_layer_set_text_color(&layer_timer_second, GColorBlack);
    text_layer_set_background_color(&layer_timer_second, GColorWhite);
}

void minute_config_view() {
    text_layer_set_background_color(&layer_timer_minute, GColorBlack);
    text_layer_set_text_color(&layer_timer_minute, GColorWhite);
    text_layer_set_background_color(&layer_timer_second, GColorWhite);
    text_layer_set_text_color(&layer_timer_second, GColorBlack);
}

void second_config_view() {
    text_layer_set_background_color(&layer_timer_minute, GColorWhite);
    text_layer_set_text_color(&layer_timer_minute, GColorBlack);
    text_layer_set_background_color(&layer_timer_second, GColorBlack);
    text_layer_set_text_color(&layer_timer_second, GColorWhite);
}



void up_click(ClickRecognizerRef recognizer, Window *window) {
    
    switch(config_mode) {
	case TIMER_CONFIG_NONE:
	    timer_toggle();
	    break;
	case TIMER_CONFIG_MINUTE:
	    if( timer_value <= 3540 )
		timer_value += 60;
	    break;
	case TIMER_CONFIG_SECOND:
	    if( timer_value < 3600 )
		timer_value += 1;
	    break;
    }

    update_display();
}

void down_click(ClickRecognizerRef recognizer, Window *window) {
    switch(config_mode) {
	case TIMER_CONFIG_NONE:
	    timer_reset();
	    break;
	case TIMER_CONFIG_MINUTE:
	    if( timer_value >= 60 )
		timer_value -= 60;
	    break;
	case TIMER_CONFIG_SECOND:
	    if( timer_value > 0 )
		timer_value -= 1;
	    break;
    }

    update_display();
}

void timer_toggle() {
    switch(timer_state) {
	case TIMER_STATE_IDLE:
	    timer_start = get_ticks_now_in_seconds();
	    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
	    timer_state = TIMER_STATE_RUNNING;
	    break;
	case TIMER_STATE_RUNNING:
	    app_timer_cancel_event(appCtx, timer_handle);
	    pause_offset += timer_elapsed;
	    timer_elapsed = 0; 
	    timer_state = TIMER_STATE_PAUSED;
	    break;
	case TIMER_STATE_PAUSED:
	    timer_start = get_ticks_now_in_seconds();
	    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
	    timer_state = TIMER_STATE_RUNNING;
	    break;
    }
}

void timer_reset() {
    timer_state = TIMER_STATE_IDLE;
    app_timer_cancel_event(appCtx, timer_handle);
    timer_elapsed = 0;
    pause_offset = 0;
    update_display();
}

void timer_handle_timer(AppContextRef ctx, AppTimerHandle handle) {
    int now = get_ticks_now_in_seconds();
    timer_elapsed  = now - timer_start;

    if( timer_elapsed + pause_offset >= timer_value ) {
	vibes_long_pulse();
	timer_reset();
	return;    
    }

    update_display();       
    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
}

void update_display() {
    convert_ticks_to_text(timer_value - pause_offset - timer_elapsed, timer_text_minute, timer_text_second);

    text_layer_set_text(&layer_timer_minute, timer_text_minute);
    text_layer_set_text(&layer_timer_second, timer_text_second);
    layer_mark_dirty(&layer_timer_minute.layer);
    layer_mark_dirty(&layer_timer_second.layer);
}

void convert_ticks_to_text(int ticks, char *minute_text, char *second_text) {

    int seconds = ticks % 60;
    int minutes = ticks / 60 % 60;

    strcpy(minute_text, itoa(minutes,2));
    strcpy(second_text, itoa(seconds,2));
}

