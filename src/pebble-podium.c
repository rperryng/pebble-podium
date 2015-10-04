#include <pebble.h>
  
#define CENTER GPoint(72, 84)

static Window *s_main_window;
static Layer *s_canvas_layer;
static GRect s_window_bounds;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_hours_buffer[] = "00";
  
  if (clock_is_24h_style()) {
    strftime(s_hours_buffer, sizeof("00"), "%H", tick_time);
  } else {
    strftime(s_hours_buffer, sizeof("00"), "%M", tick_time);
  }
  
  graphics_draw_text(
    ctx, 
    s_hours_buffer, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD), 
    GRect(0, 48, s_window_bounds.size.w, 48), 
    GTextOverflowModeWordWrap, 
    GTextAlignmentCenter, 
    NULL);
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(s_main_window);
  s_window_bounds = layer_get_bounds(root_layer);
  
  s_canvas_layer = layer_create(s_window_bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  bool animated = true;
  window_stack_push(s_main_window, animated);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
