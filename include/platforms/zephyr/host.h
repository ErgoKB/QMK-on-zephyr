#pragma once

#include "report.h"

uint8_t host_keyboard_leds(void);
void host_keyboard_send(report_keyboard_t *report);
void host_system_send(uint16_t data);
void host_consumer_send(uint16_t data);
