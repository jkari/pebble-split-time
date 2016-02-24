#pragma once

#define MESSAGE_TYPE_READY 1
#define MESSAGE_TYPE_WEATHER 2
#define MESSAGE_TYPE_CONFIG 3

void communication_init();
void communication_request_weather();
void communication_ready();