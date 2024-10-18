#include "matrix.h"
#include "zmk/split/bluetooth/key_queue.h"

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#define ROW_SIZE (sizeof(matrix_row_t) * 8)

matrix_row_t matrix[MATRIX_ROWS];

static matrix_row_t changed_pos[MATRIX_ROWS];

matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

void matrix_print(void) {}
void matrix_init(void) {}

uint8_t matrix_scan(void) {
  int32_t events = 0;
  bool changed = false;
  struct key_event ev;
  int err;

  events = k_msgq_num_used_get(&key_queue);
  if (events == 0) {
    return false;
  }

  memset(changed_pos, 0, sizeof(changed_pos));
  for (int i = 0; i < events; i++) {
    err = k_msgq_peek(&key_queue, &ev);
    if (err) {
      return changed;
    }

    int row = ev.position / ROW_SIZE;
    int col = ev.position % ROW_SIZE;
    // if this bit is changed in this loop, do not apply the change, but leave
    // it to the next matrix scan call.
    if (changed_pos[row] & BIT(col)) {
      return changed;
    }
    changed_pos[row] |= BIT(col);

    if (ev.pressed) {
      matrix[row] |= BIT(col);
    } else {
      matrix[row] &= ~((matrix_row_t)BIT(col));
    }

    // In the end, pop the top event.
    k_msgq_get(&key_queue, &ev, K_NO_WAIT);
    changed = true;
  }
  return changed;
}
