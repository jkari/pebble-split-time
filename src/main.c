#include <pebble.h>
#include "weather.h"
#include "config.h"
#include "communication.h"
#include "ui.h"

static Window *s_main_window;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Tick tock");
  
  if (tick_time->tm_min % 30 == 0) {
    weather_update();
  }
  
  ui_update_time();
}

static void battery_handler(BatteryChargeState charge_state) {
  if (charge_state.is_charging) {
    ui_battery_charge_start();
  } else {
    ui_battery_charge_stop();
  }
}

static void bluetooth_handler(bool is_connected) {
  ui_bluetooth_set_available(is_connected);
  
  vibes_double_pulse();
}

static void _focusing_handler(bool in_focus) {
  if (in_focus) {
    ui_hide();
  }
}

static void _focused_handler(bool in_focus) {
  if (in_focus) {
    ui_show();
  }
}

static void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Window load");
  
  communication_init();

  ui_load(window);
}

static void main_window_unload(Window *window) {
  ui_unload();
}

static void handle_init(void) {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  
  ui_bluetooth_set_available(connection_service_peek_pebble_app_connection());
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_handler
  });
  
  ui_update_all();
}

static void handle_deinit(void) {  
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();

  window_destroy(s_main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}