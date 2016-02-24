#pragma once

#define BATTERY_X 7
#define BATTERY_Y PBL_IF_ROUND_ELSE(-29, -19)
#define BATTERY_WIDTH 16
#define BATTERY_STROKE 3
#define BATTERY_HIGH_MIN 60
#define BATTERY_LOW_MAX 20
#define BATTERY_COLOR_HIGH GColorGreen
#define BATTERY_COLOR_MEDIUM GColorChromeYellow 
#define BATTERY_COLOR_LOW GColorRed
#define MINUTE1_RADIUS PBL_IF_ROUND_ELSE(87, 68)
#define SUN_ARC_RADIUS PBL_IF_ROUND_ELSE(76, 57)
#define MINUTE_HAND_LENGTH PBL_IF_ROUND_ELSE(60, 48)
#define HOUR_HAND_LENGTH PBL_IF_ROUND_ELSE(40, 32)
#define DATE_OFFSET_Y PBL_IF_ROUND_ELSE(-51, -45)
#define WEATHER_OFFSET_Y PBL_IF_ROUND_ELSE(29, 21)
#define BLUETOOTH_OFFSET_X PBL_IF_ROUND_ELSE(40, 32)

#define ANGLE_POINT(cx,cy,angle,radius) {.x=(int16_t)(sin_lookup(angle/360.f*TRIG_MAX_ANGLE)*radius/TRIG_MAX_RATIO)+cx,.y=(int16_t)(-cos_lookup(angle/360.f*TRIG_MAX_ANGLE)*radius/TRIG_MAX_RATIO)+cy}

void ui_bluetooth_set_available(bool is_available);
void ui_load(Window *window);
void ui_unload(void);
void ui_update_all(void);
void ui_update_time(void);
void ui_update_weather(void);
void ui_battery_charge_start(void);
void ui_battery_charge_stop(void);
void ui_set_temperature(int temperature);
void ui_set_weather_icon(uint32_t resource_id);