#include "host.h"
#include <stdint.h>

// TODO (lschyi): should return driver keyboard leds
uint8_t host_keyboard_leds(void) { return 0; }

// TODO (lschyi): implement send keyboard report
void host_keyboard_send(report_keyboard_t *report) {}
