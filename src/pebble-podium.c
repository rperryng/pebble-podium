#include <pebble.h>

#define CENTER GPoint(72, 84)

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;
static GRect s_window_bounds;
static GFont s_hours_font;
static GFont s_minutes_font;
static uint8_t s_hours_height;
static uint8_t s_minutes_height;

static bool s_animating;
static Time s_last_time;
static Time s_anim_time;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.minutes = tick_time->tm_min;

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
  graphics_context_set_text_color(ctx, GColorJazzberryJam);

  // Draw below canvas to bottom of canvas
  for (int i = 0; i < s_hours_height; i++) {
    graphics_draw_text(
      ctx,
      hours_text,
      s_hours_font,
      GRect(0, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, s_hours_height),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
  }

  // Draw from bottom of canvas to hour of day
  uint8_t padding_factor = 9;
  uint8_t length_factor = s_hours_height + ((tick_time->tm_hour * (144 - padding_factor)) / 24);
  for (int i = 0; i < length_factor; i++) {
    graphics_draw_text(
      ctx,
      hours_text,
      s_hours_font,
      GRect(0, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, s_hours_height),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
  }

  // Draw time in white.
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    hours_text,
    s_hours_font,
    GRect(0, s_window_bounds.size.h - length_factor, s_window_bounds.size.w / 2, s_hours_height),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL);
}

static void draw_minutes(GContext *ctx, struct tm *tick_time) {
  char* minutes_text = get_minutes_text(tick_time);
  graphics_context_set_text_color(ctx, GColorTiffanyBlue);

  // Draw below canvas to bottom of canvas
  for (int i = 0; i < s_minutes_height; i++) {
    graphics_draw_text(
      ctx,
      minutes_text,
      s_minutes_font,
      GRect(s_window_bounds.size.w / 2, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, s_minutes_height),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
  }

  // Draw from bottom of canvas to hour of day
  uint8_t padding_factor = 5;
  uint8_t length_factor = s_minutes_height + ((tick_time->tm_min * (144 - padding_factor)) / 60);
  for (int i = 0; i < length_factor; i++) {
    graphics_draw_text(
      ctx,
      minutes_text,
      s_minutes_font,
      GRect(s_window_bounds.size.w / 2, s_window_bounds.size.h - i, s_window_bounds.size.w / 2, s_minutes_height),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
  }

  // Draw time in white.
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    minutes_text,
    s_minutes_font,
    GRect(s_window_bounds.size.w / 2, s_window_bounds.size.h - length_factor, s_window_bounds.size.w / 2, s_minutes_height),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL);
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  graphics_context_set_antialiased(ctx, true);

  struct tm *tick_time = malloc(sizeof(struct tm));

  if (s_animating) {
    tick_time->tm_hour = s_anim_time.hours;
    tick_time->tm_min = s_anim_time.minutes;
  } else {
    tick_time->tm_hour = s_last_time.hours;
    tick_time->tm_min = s_last_time.minutes;
  }

  draw_hours(ctx, tick_time);
  draw_minutes(ctx, tick_time);
}

static void animation_on_start(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_on_end(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static int animation_percentage(AnimationProgress dist_normalized, int max) {
  return (max * dist_normalized) / ANIMATION_NORMALIZED_MAX;
}

static void animation_on_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = animation_percentage(dist_normalized, s_last_time.hours);
  s_anim_time.minutes = animation_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void animate() {
  Animation *animation = animation_create();

  if (animation) {
    animation_set_duration(animation, 2000);
    animation_set_delay(animation, 200);
    animation_
    animation_set_curve(animation, AnimationCurveEaseOut);

    static AnimationImplementation animation_implementation = {
      .update = animation_on_update
    };

    AnimationImplementation *implementation = &animation_implementation;
    animation_set_implementation(animation, implementation);
    animation_set_handlers(animation, (AnimationHandlers) {
      .started = animation_on_start,
      .stopped = animation_on_end
    }, NULL);

    animation_schedule(animation);
  }
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(s_main_window);
  s_window_bounds = layer_get_bounds(root_layer);

  s_hours_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_KEY_PROXIMA_NOVA_BOLD_58));
  s_minutes_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_KEY_PROXIMA_NOVA_REGULAR_48));
  s_hours_height = 58;
  s_minutes_height = 48;

  s_canvas_layer = layer_create(s_window_bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  time_t temp = time(NULL);
  struct tm *tm_now = localtime(&temp);
  s_last_time.hours = tm_now->tm_hour;
  s_last_time.minutes = tm_now->tm_min;

  animate();
}

static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_hours_font);
  fonts_unload_custom_font(s_minutes_font);
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
