#define STOPWATCH_TICK 59 
#define STOPWATCH_STATE_IDLE 0
#define STOPWATCH_STATE_RUNNING 1
#define STOPWATCH_STATE_PAUSED 2

void window_stopwatch_init();
void window_stopwatch_load(Window *window);
void display_laps();
void init_timer(ClickConfig **config, Window *window);
void stopwatch_config_provider(ClickConfig **config, Window *window);
void stopwatch_go(ClickRecognizerRef recognizer, Window *window);
void stopwatch_pause(ClickRecognizerRef recognizer, Window *window);
void stopwatch_reset(ClickRecognizerRef recognizer, Window *window);
void stopwatch_handle_timer(AppContextRef ctx, AppTimerHandle handle);
void convert_milliseconds_to_text();
