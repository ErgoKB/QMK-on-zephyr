#include "zmk/split/bluetooth/slot.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

struct peripheral_slot peripherals[ZMK_BLE_SPLIT_PERIPHERAL_COUNT];

bool has_open_slot(void) {
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (peripherals[i].state == PERIPHERAL_SLOT_STATE_OPEN) {
      return true;
    }
  }
  return false;
}

int reserve_peripheral_slot(const bt_addr_le_t *addr) {
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (!bt_addr_le_cmp(bt_conn_get_dst(peripherals[i].conn), addr)) {
      peripherals[i].state = PERIPHERAL_SLOT_STATE_CONNECTING;
      return i;
    }
  }

  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (peripherals[i].state == PERIPHERAL_SLOT_STATE_OPEN) {
      peripherals[i].state = PERIPHERAL_SLOT_STATE_CONNECTING;
      return i;
    }
  }
  return -ENOMEM;
}

void confirm_peripheral_slot_conn(struct bt_conn *conn) {
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (peripherals[i].conn == conn) {
      peripherals[i].state = PERIPHERAL_SLOT_STATE_CONNECTED;
      return;
    }
  }
}

struct peripheral_slot *get_slot_by_connection(struct bt_conn *conn) {
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (peripherals[i].conn == conn) {
      return &peripherals[i];
    }
  }
  return NULL;
}

void release_peripheral_slot(int index) {
  struct peripheral_slot *slot = &peripherals[index];

  if (slot->conn != NULL) {
    bt_conn_unref(slot->conn);
    slot->conn = NULL;
  }
  slot->state = PERIPHERAL_SLOT_STATE_OPEN;

  // TODO (lschyi)
  // Raise events releasing any active positions from this peripheral

  for (int i = 0; i < POSITION_STATE_DATA_LEN; i++) {
    slot->position_state[i] = 0U;
    slot->changed_positions[i] = 0U;
  }

  // Clean up previously discovered handles;
  slot->subscribe_params.value_handle = 0;
  slot->run_behavior_handle = 0;
}

void release_peripheral_slot_for_conn(struct bt_conn *conn) {
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (peripherals[i].conn == conn) {
      return release_peripheral_slot(i);
    }
  }
}
