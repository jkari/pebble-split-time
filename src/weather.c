#include <pebble.h>
#include "weather.h"
#include "config.h"
#include "communication.h"
#include "ui.h"

void weather_init(void) {
  if (!persist_exists(PERSIST_KEY_TEMPERATURE)) {
    weather_update();
  }
}

int weather_get_resource_id(int condition) {
  // http://openweathermap.org/weather-conditions
  int resource_id = 0;
  
  if (condition == 200 || condition == 210 || condition == 230) {
    resource_id = RESOURCE_ID_IMAGE_THUNDER_LIGHT;
  } else if (condition >= 200 && condition < 300) {
    resource_id = RESOURCE_ID_IMAGE_THUNDER_HEAVY;
  } else if (condition == 300 || condition == 310 || condition == 500 || condition == 520) {
    resource_id = RESOURCE_ID_IMAGE_RAIN_LIGHT;
  } else if (condition >= 300 && condition < 600) {
    resource_id = RESOURCE_ID_IMAGE_RAIN_HEAVY;
  } else if (condition == 600 || condition == 615 || condition == 620) {
    resource_id = RESOURCE_ID_IMAGE_SNOW_LIGHT;
  } else if (condition == 621 || condition == 622) {
    resource_id = RESOURCE_ID_IMAGE_SNOW_SHOWER;
  } else if (condition >= 600 && condition < 700) {
    resource_id = RESOURCE_ID_IMAGE_SNOW_HEAVY;
  } else if (condition >= 700 && condition < 800) {
    resource_id = RESOURCE_ID_IMAGE_MIST;
  } else if (condition == 800) {
    resource_id = RESOURCE_ID_IMAGE_CLEAR;
  } else if (condition == 801) {
    resource_id = RESOURCE_ID_IMAGE_CLOUDS_LIGHT;
  } else if (condition == 802) {
    resource_id = RESOURCE_ID_IMAGE_CLOUDS_MEDIUM;
  } else if (condition == 803 || condition == 804) {
    resource_id = RESOURCE_ID_IMAGE_CLOUDS_HEAVY;
  }
  
  return resource_id;
}

void weather_update(void) {
  communication_request_weather();
}

void weather_received_callback(DictionaryIterator* iterator) {
  Tuple *temperature_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  Tuple *sunrise_tuple = dict_find(iterator, KEY_SUNRISE);
  Tuple *sunset_tuple = dict_find(iterator, KEY_SUNSET);
 
  if (temperature_tuple && conditions_tuple && sunrise_tuple && sunset_tuple) {
    persist_write_int(PERSIST_KEY_TEMPERATURE, (int)temperature_tuple->value->int32);
    persist_write_int(PERSIST_KEY_CONDITION, (int)conditions_tuple->value->int32);
    persist_write_int(PERSIST_KEY_SUNRISE, (int)sunrise_tuple->value->int32);
    persist_write_int(PERSIST_KEY_SUNSET, (int)sunset_tuple->value->int32);
  }
  
  ui_update_weather();
}

int weather_get_sunrise_hour() {
  return persist_read_int(PERSIST_KEY_SUNRISE) / 100;
}

int weather_get_sunset_hour() {
  return persist_read_int(PERSIST_KEY_SUNSET) / 100;
}

int weather_get_sunrise_minute() {
  return persist_read_int(PERSIST_KEY_SUNRISE) % 100;
}

int weather_get_sunset_minute() {
  return persist_read_int(PERSIST_KEY_SUNSET) % 100;
}

int weather_get_condition() {
  return persist_read_int(PERSIST_KEY_CONDITION);
}

int weather_get_temperature() {
  return persist_read_int(PERSIST_KEY_TEMPERATURE);
}