#include "host.h"
#include "report.h"
#include "usb_device_state.h"
#include "usb_main.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(qmk_on_zephyr);

#define SW0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

void protocol_setup(void) { usb_device_state_init(); }

void protocol_pre_init(void) { init_usb_driver(); }
void protocol_post_init(void) {}

void protocol_pre_task(void) {
  static int counter = 0;
  static int val = 0;
  static report_keyboard_t kb_report = {
      .keys = {0},
  };

  int new_val = gpio_pin_get_dt(&button);
  if (new_val != val) {
    val = new_val;
    kb_report.keys[0] = (new_val) ? 4 : 0;
    host_keyboard_send(&kb_report);
    if (new_val) {
      ++counter;
      printf("counter: %d\n", counter);
    }
  }
}
void protocol_post_task(void) {}

void platform_setup(void) {
  // Hardware initialization.
  int ret;

  if (!device_is_ready(button.port)) {
    printk("Error: button device %s is not ready\n", button.port->name);
    return;
  }

  ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
  if (ret != 0) {
    printk("Error %d: failed to configure %s pin %d\n", ret, button.port->name,
           button.pin);
    return;
  }
}
