#pragma once

int weather_get_sunrise_hour(void);
int weather_get_sunset_hour(void);
int weather_get_sunrise_minute(void);
int weather_get_sunset_minute(void);
int weather_get_condition(void);
int weather_get_temperature(void);
void weather_init(void);
void weather_update(void);
void weather_received_callback(DictionaryIterator* iterator);
int weather_get_resource_id(int condition);