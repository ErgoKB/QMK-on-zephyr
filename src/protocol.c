#include "host.h"
#include "report.h"
#include "usb_device_state.h"
#include "usb_main.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(qmk_on_zephyr);

void protocol_setup(void) { usb_device_state_init(); }

void protocol_pre_init(void) { init_usb_driver(); }
void protocol_post_init(void) {}

void protocol_pre_task(void) {
#if IS_ENABLED(CONFIG_BT)
  // Proactively yield itself for cooperative multitasking. Give away CPU time
  // for BT and other threads, such that BT SMP can finish pairing.
  k_usleep(1);
#endif /* IS_ENABLED(CONFIG_BT) */
}
void protocol_post_task(void) {
#if CONFIG_RAW_ENABLE
  raw_hid_task();
#endif /* CONFIG_RAW_ENABLE */

#if IS_ENABLED(CONFIG_BT)
  // Proactively yield itself for cooperative multitasking. Give away CPU time
  // for BT and other threads, such that BT SMP can finish pairing.
  k_usleep(1);
#endif /* IS_ENABLED(CONFIG_BT) */
}

void platform_setup(void) {
  // Hardware initialization.
}
