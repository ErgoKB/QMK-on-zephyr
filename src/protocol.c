#include "host.h"
#include "report.h"
#include "usb_device_state.h"
#include "usb_main.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(qmk_on_zephyr);

void protocol_setup(void) { usb_device_state_init(); }

void protocol_pre_init(void) { init_usb_driver(); }
void protocol_post_init(void) {}

void protocol_pre_task(void) {}
void protocol_post_task(void) {}

void platform_setup(void) {
  // Hardware initialization.
}
