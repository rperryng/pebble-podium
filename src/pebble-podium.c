#include <pebble.h>
  
#define CENTER GPoint(72, 84)

static Window *s_main_window;
static Layer *s_canvas_layer;
static GRect s_window_bounds;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static char* get_hours_text(struct tm *tick_time) {
  static char s_hours_buffer[] = "00";
  
  if (clock_is_24h_style()) {
    strftime(s_hours_buffer, sizeof("00"), "%H", tick_time);
  } else {
    strftime(s_hours_buffer, sizeof("00"), "%I", tick_time);
  }
  
  return s_hours_buffer;
}

static char* get_minutes_text(struct tm *tick_time) {
  static char s_minutes_buffer[] = "00"; 
  strftime(s_minutes_buffer, sizeof("00"), "%M", tick_time);
  return s_minutes_buffer;
}

static void draw_hours(GContext *ctx, struct tm *tick_time) {
  char* hours_text = get_hours_text(tick_time);
  GFont hours_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  graphics_context_set_text_color(ctx, GColorJazzberryJam);
  
  // Draw below canvas to bottom of canvas
  for (int i = 0; i < 42; i++) {
    graphics_draw_text(
      ctx,
      hours_text,
      hours_font,
      GRect(0, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, 42),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);  
  }
  
  // Draw from bottom of canvas to hour of day
  uint8_t length_factor = ((tick_time->tm_hour * 144) / 24) + 42;
  for (int i = 0; i < length_factor; i++) {
    graphics_draw_text(
      ctx,
      hours_text,
      hours_font,
      GRect(0, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, 34),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
  }
  
  // Draw time in white.
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    hours_text,
    hours_font,
    GRect(0, s_window_bounds.size.h - length_factor, s_window_bounds.size.w / 2, 34),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL);
}

static void draw_minutes(GContext *ctx, struct tm *tick_time) {
  char* minutes_text = get_minutes_text(tick_time);
  GFont minutes_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  graphics_context_set_text_color(ctx, GColorTiffanyBlue); 
  
  // Draw below canvas to bottom of canvas
  for (int i = 0; i < 34; i++) {
    graphics_draw_text(
      ctx,
      minutes_text,
      minutes_font,
      GRect(s_window_bounds.size.w / 2, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, 34),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);  
  }
  
  // Draw from bottom of canvas to hour of day
  uint8_t length_factor = ((tick_time->tm_min * 144) / 60) + 42;
  for (int i = 0; i < length_factor; i++) {
    graphics_draw_text(
      ctx,
      minutes_text,
      minutes_font,
      GRect(s_window_bounds.size.w / 2, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, 34),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
  }
  
  // Draw time in white.
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    minutes_text,
    minutes_font,
    GRect(s_window_bounds.size.w / 2, s_window_bounds.size.h - length_factor, s_window_bounds.size.w / 2, 34),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL);
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  draw_hours(ctx, tick_time);
  draw_minutes(ctx, tick_time);
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
