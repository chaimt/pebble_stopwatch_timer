#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "app.h"
#include "stopwatch.h"
#include "timer.h"

#define MY_UUID { 0x8E, 0x14, 0x4A, 0xE3, 0x6C, 0xA7, 0x4B, 0xFA, 0x9C, 0x41, 0xB6, 0x67, 0xAC, 0xE7, 0x72, 0x0E }
PBL_APP_INFO(MY_UUID,
	"Stopwatch/Timer", "nalcire",
	1, 0, /* App version */
	DEFAULT_MENU_ICON,
	APP_INFO_STANDARD_APP);

AppContextRef appCtx;

void handle_init(AppContextRef ctx) {
    (void)ctx;
    appCtx = ctx;
    window_stopwatch_init();
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
    switch( cookie ) {
	case COOKIE_TIMER:
	    timer_handle_timer(ctx, handle);
	    break;
	case COOKIE_STOPWATCH:
	    stopwatch_handle_timer(ctx, handle);
	    break;
    }
}

void pbl_main(void *params) {
    PebbleAppHandlers handlers = {
	.init_handler = &handle_init,
	.timer_handler = &handle_timer
    };
    app_event_loop(params, &handlers);
}
