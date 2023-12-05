#include "host.h"
#include "report.h"
#include "usb_main.h"

#include <stdint.h>

// TODO (lschyi): should return driver keyboard leds
uint8_t host_keyboard_leds(void) { return 0; }

void host_keyboard_send(report_keyboard_t *report) {
  zephyr_driver.send_keyboard(report);
}
