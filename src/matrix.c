#include "matrix.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define SW0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

void matrix_init_custom(void) {
  int ret;
  printk("custom matrix init");

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

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
  static int last_val;
  int val = gpio_pin_get_dt(&button);
  if (val != last_val) {
      current_matrix[0] = (val) ? 1 : 0;
      last_val = val;
      return true;
  }
  return false;
}
