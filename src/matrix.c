#include "matrix.h"
#include "zmk/split/bluetooth/key_queue.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define ROW_SIZE (sizeof(matrix_row_t) * 8)

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
  bool changed = false;
  struct key_event ev;
  int err;
  while (true) {
    err = k_msgq_get(&key_queue, &ev, K_NO_WAIT);
    if (err) {
      break;
    }
    changed = true;
    int row = ev.position / ROW_SIZE;
    int col = ev.position % ROW_SIZE;
    if (ev.pressed) {
      current_matrix[row] |= BIT(col);
    } else {
      current_matrix[row] &= ~((matrix_row_t)BIT(col));
    }
  }
  return changed;
}
