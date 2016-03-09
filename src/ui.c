#include <pebble.h>
#include "config.h"
#include "weather.h"
#include "ui.h"
#include "gbitmap_color_palette_manipulator.h"

static TextLayer *s_layer_temperature;
static TextLayer *s_layer_day_of_month;
static TextLayer *s_layer_weekday;
static Layer *s_layer_hands_left;
static Layer *s_layer_hands_right;
static Layer *s_layer_bg_left;
static Layer *s_layer_bg_right;
static Layer *s_layer_battery;
static GBitmap *s_bitmap_weather = 0;
static GBitmap *s_bitmap_bluetooth = 0;
static GBitmap *s_bitmap_sun_left = 0;
static GBitmap *s_bitmap_sun_right = 0;
static GBitmap *s_bitmap_moon_left = 0;
static GBitmap *s_bitmap_moon_right = 0;
static BitmapLayer *s_layer_bluetooth;
static BitmapLayer *s_layer_weather;
static GFont s_font_huge;
static GFont s_font_big;
static GFont s_font_small;
static GFont s_font_pixel;
static GFont s_font_custom;
static bool is_battery_animation_active = false;
static int battery_animation_percent = 0;

static void _ui_set_temperature() {
  static char temperature_buffer[8];
  
  int temperature = weather_get_temperature();
  
  if (temperature >= 0) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "+%d", temperature);
  } else {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", temperature);
  }
  
  text_layer_set_text(s_layer_temperature, temperature_buffer);
}

static void _ui_reload_bitmap(GBitmap **image, uint32_t resource_id, GColor color) {
  if (*image == 0) {
    gbitmap_destroy(*image);
  }
  
  *image = gbitmap_create_with_resource(resource_id);
  
  replace_gbitmap_color(GColorWhite, color, *image, NULL);
}

static void _ui_set_weather_icon() {
  int32_t resource_id = weather_get_resource_id(weather_get_condition());
  
  _ui_reload_bitmap(&s_bitmap_weather, resource_id, config_get_pointer_color_right());
  bitmap_layer_set_bitmap(s_layer_weather, s_bitmap_weather);
  
  layer_set_hidden((Layer *)s_layer_weather, false);
}

static void _generate_bitmaps() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Generating bitmaps");
  _ui_set_weather_icon();
  _ui_reload_bitmap(&s_bitmap_sun_left, RESOURCE_ID_IMAGE_SUN, config_get_color_right());
  _ui_reload_bitmap(&s_bitmap_sun_right, RESOURCE_ID_IMAGE_SUN, config_get_color_left());
  _ui_reload_bitmap(&s_bitmap_moon_left, RESOURCE_ID_IMAGE_MOON, config_get_color_right());
  _ui_reload_bitmap(&s_bitmap_moon_right, RESOURCE_ID_IMAGE_MOON, config_get_color_left());
}

static void _charge_animation_callback(void *data) {
  battery_animation_percent = (battery_animation_percent + 2) % 100;
  
  if (is_battery_animation_active) {
    app_timer_register(30, _charge_animation_callback, NULL);
  }
  
  layer_mark_dirty(s_layer_battery);
}

static void _draw_sun_cycle(GContext *ctx, GPoint offset, bool isLeft) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  GBitmap *bitmap_sun = isLeft ? s_bitmap_sun_left : s_bitmap_sun_right;
  GBitmap *bitmap_moon = isLeft ? s_bitmap_moon_left : s_bitmap_moon_right;
  
  float hour_sunrise = weather_get_sunrise_hour() + weather_get_sunrise_minute() / 60.f;
  float hour_sunset = weather_get_sunset_hour() + weather_get_sunset_minute() / 60.f;
  bool is_min_12_light_hours = (hour_sunrise > hour_sunset && hour_sunset + 24 - hour_sunrise >= 12)
    || hour_sunset - hour_sunrise >= 12;
  
  hour_sunrise = (hour_sunrise >= 13) ? hour_sunrise - 12 : hour_sunrise;
  hour_sunset = (hour_sunset >= 13) ? hour_sunset - 12 : hour_sunset;
  
  float angle_start = 360.f * hour_sunrise / 12.f;
  float angle_end = 360.f * hour_sunset / 12.f;
  
  GPoint start = ANGLE_POINT(offset.x, offset.y, angle_start, SUN_ARC_RADIUS);
  GPoint end = ANGLE_POINT(offset.x, offset.y, angle_end, SUN_ARC_RADIUS);
  graphics_draw_bitmap_in_rect(ctx, bitmap_sun, GRect(start.x - 10, start.y - 10, 20, 20));
  graphics_draw_bitmap_in_rect(ctx, bitmap_moon, GRect(end.x - 10, end.y - 10, 20, 20));
  
  if (is_min_12_light_hours) {
    float temp = angle_start;
    angle_start = angle_end;
    angle_end = temp;
  }
  
  if (angle_end < angle_start) {
    angle_end += 360.f;
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Angle %d - %d, isLeft: %d", (int)angle_start, (int)angle_end, (int)isLeft);
  APP_LOG(APP_LOG_LEVEL_INFO, "Color %d",isLeft ? config_get_color_right().argb : config_get_color_left().argb);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, isLeft ? config_get_color_right() : config_get_color_left());
  graphics_draw_arc(
    ctx,
    GRect(
      offset.x - SUN_ARC_RADIUS,
      offset.y - SUN_ARC_RADIUS,
      2*SUN_ARC_RADIUS, 2*SUN_ARC_RADIUS
    ),
    GOvalScaleModeFitCircle,
    DEG_TO_TRIGANGLE(angle_start + 7),
    DEG_TO_TRIGANGLE(angle_end - 7)
  );
}

static void _draw_bg(GContext *ctx, GPoint offset, GRect area, bool isLeft) {
  GColor dotColor = isLeft ? config_get_color_right() : config_get_color_left();
  GColor bgColor = isLeft ? config_get_pointer_color_left() : config_get_pointer_color_right();
  
  graphics_context_set_fill_color(ctx, bgColor);
  graphics_fill_rect(ctx, area, 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, dotColor);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  for (int i = (isLeft ? 30 : 0); i <= (isLeft ? 60 : 30); i++) {
    GPoint center = ANGLE_POINT(offset.x, offset.y, 360 * i / 60, MINUTE1_RADIUS);
    graphics_fill_circle(ctx, center, i % 5 == 0 ? 3 : 1);
  }
  
  _draw_sun_cycle(ctx, offset, isLeft);
}

static void _draw_hands(GContext *ctx, GPoint offset, bool isDark) {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  GColor handColor = isDark ? config_get_color_right() : config_get_color_left();
  GColor pointerColor = isDark ? config_get_pointer_color_right() : config_get_pointer_color_left();
  
  int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
  GPoint minute_hand_end = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)MINUTE_HAND_LENGTH / TRIG_MAX_RATIO) + offset.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)MINUTE_HAND_LENGTH / TRIG_MAX_RATIO) + offset.y,
  };
  GPoint minute_pointer_end = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)MINUTE_POINTER_LENGTH / TRIG_MAX_RATIO) + minute_hand_end.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)MINUTE_POINTER_LENGTH / TRIG_MAX_RATIO) + minute_hand_end.y,
  };
  
  int32_t hour_angle = TRIG_MAX_ANGLE * ((tick_time->tm_hour % 12) / 12.f + tick_time->tm_min / (12.f * 60));
  GPoint hour_hand_end = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)HOUR_HAND_LENGTH / TRIG_MAX_RATIO) + offset.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)HOUR_HAND_LENGTH / TRIG_MAX_RATIO) + offset.y,
  };
  GPoint hour_pointer_end = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)HOUR_POINTER_LENGTH / TRIG_MAX_RATIO) + hour_hand_end.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)HOUR_POINTER_LENGTH / TRIG_MAX_RATIO) + hour_hand_end.y,
  };
  
  // minute
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, handColor);
  graphics_draw_line(ctx, offset, minute_hand_end);
  
  // minute pointer
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_stroke_color(ctx, pointerColor);
  graphics_draw_line(ctx, minute_hand_end, minute_pointer_end);
  
  // hour
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, handColor);
  graphics_draw_line(ctx, offset, hour_hand_end);
  
  // hour pointer
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_stroke_color(ctx, pointerColor);
  graphics_draw_line(ctx, hour_hand_end, hour_pointer_end);
}

static void _layer_bg_left_update_callback(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update left bg");
  GRect bounds = layer_get_bounds(layer);
  _draw_bg(ctx, GPoint(bounds.size.w, bounds.size.h / 2), bounds, true);
}

static void _layer_bg_right_update_callback(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update right bg");
  GRect bounds = layer_get_bounds(layer);
  _draw_bg(ctx, GPoint(0, bounds.size.h / 2), bounds, false);
}

static void _layer_hands_left_update_callback(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update left hands");
  GRect bounds = layer_get_bounds(layer);
  _draw_hands(ctx, GPoint(bounds.size.w, bounds.size.h / 2), true);
}

static void _layer_hands_right_update_callback(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update right hands");
  GRect bounds = layer_get_bounds(layer);
  _draw_hands(ctx, GPoint(0, bounds.size.h / 2), false);
}

static void _layer_battery_update_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  BatteryChargeState charge_state = battery_state_service_peek();
  int current_charge = is_battery_animation_active ? battery_animation_percent : charge_state.charge_percent;

  graphics_context_set_stroke_color(ctx, config_get_color_left());
  graphics_context_set_stroke_width(ctx, BATTERY_STROKE + 2);
  graphics_draw_line(ctx, GPoint(BATTERY_X, center.y + BATTERY_Y), GPoint(BATTERY_X + BATTERY_WIDTH, center.y + BATTERY_Y));
  
  graphics_context_set_stroke_color(ctx, config_get_color_right());
  graphics_context_set_stroke_width(ctx, BATTERY_STROKE);
  graphics_draw_line(ctx, GPoint(BATTERY_X, center.y + BATTERY_Y), GPoint(BATTERY_X + BATTERY_WIDTH, center.y + BATTERY_Y));
  
  GColor batteryColor = BATTERY_COLOR_MEDIUM;
  if (charge_state.charge_percent <= BATTERY_LOW_MAX) {
    batteryColor = BATTERY_COLOR_LOW;
  } else if (charge_state.charge_percent >= BATTERY_HIGH_MIN) {
    batteryColor = BATTERY_COLOR_HIGH;
  }
  graphics_context_set_stroke_color(ctx, batteryColor);
  graphics_draw_line(ctx, GPoint(BATTERY_X, center.y + BATTERY_Y), GPoint(BATTERY_X + (current_charge / 100.0) * BATTERY_WIDTH, center.y + BATTERY_Y));
}

static void _update_date(void) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_day_of_month_buffer[6];
  strftime(s_day_of_month_buffer, sizeof(s_day_of_month_buffer), "%d", tick_time);
    
  static char s_weekday_buffer[4];
  strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%a", tick_time);
  
  text_layer_set_text(s_layer_day_of_month, s_day_of_month_buffer);
  text_layer_set_text(s_layer_weekday, s_weekday_buffer);
}

void ui_load(Window *window) {
                              
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);

  s_font_huge = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_24));
  s_font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_16));
  s_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_12));
  s_font_pixel = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PIXEL_16));
  s_font_custom = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_20));
  
  s_bitmap_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  
  s_layer_day_of_month = text_layer_create(GRect(20, center.y - 18, 40, 40));
  text_layer_set_background_color(s_layer_day_of_month, GColorClear);
  text_layer_set_font(s_layer_day_of_month, s_font_custom);
  text_layer_set_text_alignment(s_layer_day_of_month, GTextAlignmentLeft);
  
  s_layer_weekday = text_layer_create(GRect(20, center.y + 3, 40, 40));
  text_layer_set_background_color(s_layer_weekday, GColorClear);
  text_layer_set_font(s_layer_weekday, s_font_custom);
  text_layer_set_text_alignment(s_layer_weekday, GTextAlignmentLeft);

  s_layer_temperature = text_layer_create(GRect(center.x - 48, center.y - 18, 32, 32));
  text_layer_set_background_color(s_layer_temperature, GColorClear);
  text_layer_set_text_alignment(s_layer_temperature, GTextAlignmentCenter);
  text_layer_set_font(s_layer_temperature, s_font_custom);
  
  s_layer_weather = bitmap_layer_create(GRect(center.x - 46, center.y + 4 - 3, 32, 32));
  bitmap_layer_set_compositing_mode(s_layer_weather, GCompOpSet);
  bitmap_layer_set_bitmap(s_layer_weather, s_bitmap_weather);
  layer_set_hidden((Layer *)s_layer_weather, true);
  
  s_layer_bluetooth = bitmap_layer_create(GRect(center.x + BLUETOOTH_OFFSET_X, center.y - 10, 20, 20));
  bitmap_layer_set_compositing_mode(s_layer_bluetooth, GCompOpSet);
  bitmap_layer_set_bitmap(s_layer_bluetooth, s_bitmap_bluetooth);
  
  s_layer_bg_left = layer_create(GRect(0, 0, bounds.size.w / 2, bounds.size.h));
  layer_set_update_proc(s_layer_bg_left, _layer_bg_left_update_callback);
  
  s_layer_bg_right = layer_create(GRect(bounds.size.w / 2, 0, bounds.size.w / 2, bounds.size.h));
  layer_set_update_proc(s_layer_bg_right, _layer_bg_right_update_callback);
  
  s_layer_hands_left = layer_create(GRect(0, 0, bounds.size.w / 2, bounds.size.h));
  layer_set_update_proc(s_layer_hands_left, _layer_hands_left_update_callback);
  
  s_layer_hands_right = layer_create(GRect(0, 0, bounds.size.w / 2, bounds.size.h));
  layer_set_update_proc(s_layer_hands_right, _layer_hands_right_update_callback);
  
  s_layer_battery = layer_create(GRect(0, 0, bounds.size.w / 2, bounds.size.h));
  layer_set_update_proc(s_layer_battery, _layer_battery_update_callback);
    
  layer_add_child(window_layer, s_layer_bg_left);
  layer_add_child(window_layer, s_layer_bg_right);
  
  layer_add_child(s_layer_bg_right, text_layer_get_layer(s_layer_day_of_month));
  layer_add_child(s_layer_bg_right, text_layer_get_layer(s_layer_weekday));
  layer_add_child(s_layer_bg_left, text_layer_get_layer(s_layer_temperature));
  
  layer_add_child(s_layer_bg_left, bitmap_layer_get_layer(s_layer_weather));
  layer_add_child(s_layer_bg_right, bitmap_layer_get_layer(s_layer_bluetooth));
  
  layer_add_child(s_layer_bg_right, s_layer_battery);
  layer_add_child(s_layer_bg_left, s_layer_hands_left);
  layer_add_child(s_layer_bg_right, s_layer_hands_right);
}

void ui_unload(void) {
  text_layer_destroy(s_layer_day_of_month);
  text_layer_destroy(s_layer_weekday);
  text_layer_destroy(s_layer_temperature);
  
  bitmap_layer_destroy(s_layer_weather);
  bitmap_layer_destroy(s_layer_bluetooth);
  
  layer_destroy(s_layer_battery);
  layer_destroy(s_layer_hands_left);
  layer_destroy(s_layer_hands_right);
  layer_destroy(s_layer_bg_left);
  layer_destroy(s_layer_bg_right);
  
  fonts_unload_custom_font(s_font_big);
  fonts_unload_custom_font(s_font_small);
  
  gbitmap_destroy(s_bitmap_weather);
  gbitmap_destroy(s_bitmap_bluetooth);
  gbitmap_destroy(s_bitmap_sun_left);
  gbitmap_destroy(s_bitmap_sun_right);
  gbitmap_destroy(s_bitmap_moon_left);
  gbitmap_destroy(s_bitmap_moon_right);
}

void ui_update_time(void) {
  _update_date();
  layer_mark_dirty(s_layer_hands_left);
  layer_mark_dirty(s_layer_hands_right);
}

void ui_update_all(void) {
  
  text_layer_set_text_color(s_layer_temperature, config_get_pointer_color_right());
  text_layer_set_text_color(s_layer_weekday, config_get_color_left());
  text_layer_set_text_color(s_layer_day_of_month, config_get_color_left());
  
  ui_update_weather();
  
  layer_mark_dirty(s_layer_bg_left);
  layer_mark_dirty(s_layer_bg_right);
}

void ui_bluetooth_set_available(bool is_available) {
  layer_set_hidden((Layer *)s_layer_bluetooth, is_available);
}

void ui_battery_charge_start(void) {
  is_battery_animation_active = true;
  _charge_animation_callback(NULL);
}

void ui_battery_charge_stop(void) {
  is_battery_animation_active = false;
  layer_mark_dirty(s_layer_battery);
}

void ui_update_weather() {
  _generate_bitmaps();

  _ui_set_temperature();
  _ui_set_weather_icon();
}

void ui_show() {  
  Layer* layer_root = window_get_root_layer(layer_get_window(s_layer_bg_left));
  layer_set_hidden(layer_root, false);
  layer_mark_dirty(layer_root);
}

void ui_hide() {
  Layer* layer_root = window_get_root_layer(layer_get_window(s_layer_bg_left));
  layer_set_hidden(layer_root, true);
}