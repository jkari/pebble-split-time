#include <pebble.h>
#include "ui.h"
#include "weather.h"
#include "config.h"

static void _update_settings(int use_celcius, int color_left, int color_right) {
  persist_write_int(PERSIST_KEY_USE_CELCIUS, use_celcius);
  persist_write_int(PERSIST_KEY_COLOR_LEFT, color_left);
  persist_write_int(PERSIST_KEY_COLOR_RIGHT, color_right);
  
  ui_update_all();
  weather_update();
}

void config_received_callback(DictionaryIterator* iterator) {
  Tuple *use_celcius = dict_find(iterator, KEY_USE_CELCIUS);
  Tuple *color_left = dict_find(iterator, KEY_COLOR_LEFT);
  Tuple *color_right = dict_find(iterator, KEY_COLOR_RIGHT);
  
  if (use_celcius && color_left && color_right) {
    _update_settings(use_celcius->value->int32, color_left->value->int32, color_right->value->int32);  
  }
}

GColor config_get_color_left() {
  if (persist_exists(PERSIST_KEY_COLOR_LEFT)) {
    return GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_LEFT));
  }
  
  return GColorFromRGBA(156,156,156,255);
}

GColor config_get_color_right() {
  if (persist_exists(PERSIST_KEY_COLOR_RIGHT)) {
    return GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_RIGHT));
  }
  
  return GColorFromRGBA(128,128,128,255);
}

GColor config_get_pointer_color_left() {
  return GColorFromRGBA(255,255,255,255);
  
}

GColor config_get_pointer_color_right() {
  return GColorFromRGBA(0,0,0,255);  
}

bool config_get_use_celcius() {
  if (persist_exists(PERSIST_KEY_USE_CELCIUS)) {
    return persist_read_int(PERSIST_KEY_USE_CELCIUS) > 0;
  }
  
  return true;
}