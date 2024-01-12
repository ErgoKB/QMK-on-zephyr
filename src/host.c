#include "host.h"
#include "report.h"
#include "usb_main.h"

#include <stdint.h>

static uint16_t last_system_usage = 0;
static uint16_t last_consumer_usage = 0;
// TODO (lschyi): should return driver keyboard leds
uint8_t host_keyboard_leds(void) { return 0; }

void host_keyboard_send(report_keyboard_t *report) {
  zephyr_driver.send_keyboard(report);
}

void host_mouse_send(report_mouse_t *report) {
  zephyr_driver.send_mouse(report);
}

void host_system_send(uint16_t usage) {
  if (usage == last_system_usage)
    return;
  last_system_usage = usage;

  report_extra_t report = {
      .report_id = REPORT_ID_SYSTEM,
      .usage = usage,
  };
  zephyr_driver.send_extra(&report);
}

void host_consumer_send(uint16_t usage) {
  if (usage == last_consumer_usage)
    return;
  last_consumer_usage = usage;

  report_extra_t report = {
      .report_id = REPORT_ID_CONSUMER,
      .usage = usage,
  };
  zephyr_driver.send_extra(&report);
}
