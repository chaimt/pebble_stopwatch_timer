#define TIMER_DEFAULT 30

#define TIMER_TICK 59 

#define TIMER_STATE_IDLE 0
#define TIMER_STATE_RUNNING 1
#define TIMER_STATE_PAUSED 2

#define TIMER_CONFIG_NONE 0
#define TIMER_CONFIG_MINUTE 1
#define TIMER_CONFIG_SECOND 2

void window_timer_init();
void window_timer_load(Window *window);
void timer_config_provider(ClickConfig **config, Window *window);
void down_click(ClickRecognizerRef recognizer, Window *window);
void up_click(ClickRecognizerRef recognizer, Window *window);
void timer_config(ClickRecognizerRef recognizer, Window *window);
void timer_config_stop(ClickRecognizerRef recognizer, Window *window);
void minute_config_view();
void second_config_view();
void normal_view();
void timer_toggle();
void timer_reset();
void timer_handle_timer(AppContextRef ctx, AppTimerHandle handle);
void update_display();
void convert_ticks_to_text(int seconds, char *minute_text, char *second_text);
