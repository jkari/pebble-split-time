#include <pebble.h>
#include "communication.h"
#include "config.h"
#include "weather.h"

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *message_type = dict_find(iterator, KEY_MESSAGE_TYPE);

  switch (message_type->value->int32) {
    case MESSAGE_TYPE_READY:
      communication_ready();
      break;
    case MESSAGE_TYPE_WEATHER:
      weather_received_callback(iterator);
      break;
    case MESSAGE_TYPE_CONFIG:
      config_received_callback(iterator);
      break;
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void communication_init() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Initializing communication");
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(1024, 64);
}

void communication_request_weather() {
  int use_celcius = config_get_use_celcius() ? 1 : 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "Sending weather request");
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, KEY_USE_CELCIUS, &use_celcius, sizeof(int), true);
  app_message_outbox_send();
}

void communication_ready() {
  weather_init();
}