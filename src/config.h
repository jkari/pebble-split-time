#pragma once

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_SUNRISE 2
#define KEY_SUNSET 3
#define KEY_USE_CELCIUS 4
#define KEY_COLOR_LEFT 5
#define KEY_COLOR_RIGHT 6
#define KEY_MESSAGE_TYPE 7

#define PERSIST_KEY_TEMPERATURE 1
#define PERSIST_KEY_CONDITION 2
#define PERSIST_KEY_SUNRISE 3
#define PERSIST_KEY_SUNSET 4
#define PERSIST_KEY_USE_CELCIUS 5
#define PERSIST_KEY_COLOR_LEFT 6
#define PERSIST_KEY_COLOR_RIGHT 7

void config_received_callback(DictionaryIterator* iterator);
GColor config_get_color_left();
GColor config_get_color_right();
bool config_get_use_celcius();