#include "matrix.h"
#include "zmk/split/bluetooth/key_queue.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#define ROW_SIZE (sizeof(matrix_row_t) * 8)

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
