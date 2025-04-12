#include "zmk/split/bluetooth/slot.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

struct peripheral_slot peripherals[ZMK_BLE_SPLIT_PERIPHERAL_COUNT];

K_MUTEX_DEFINE(slot_mutex);

bool has_open_slot(void) {
  bool ret = false;

  k_mutex_lock(&slot_mutex, K_FOREVER);
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (!peripherals[i].occupied) {
      ret = true;
      goto exit_has_open_slot;
    }
  }
exit_has_open_slot:
  k_mutex_unlock(&slot_mutex);
  return ret;
}

int request_peripheral_slot(const bt_addr_le_t *addr,
                            struct peripheral_slot **slot) {
  int ret = 0;

  k_mutex_lock(&slot_mutex, K_FOREVER);
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (!bt_addr_le_cmp(bt_conn_get_dst(peripherals[i].conn), addr)) {
      *slot = &peripherals[i];
      goto exit_reserve_peripheral_slot;
    }
  }

  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (!peripherals[i].occupied) {
      peripherals[i].occupied = true;
      *slot = &peripherals[i];
      goto exit_reserve_peripheral_slot;
    }
  }

  ret = -ENOMEM;
exit_reserve_peripheral_slot:
  k_mutex_unlock(&slot_mutex);
  return ret;
}

int get_slot_by_connection(struct bt_conn *conn,
                           struct peripheral_slot **slot) {
  int res = 0;

  k_mutex_lock(&slot_mutex, K_FOREVER);
  for (int i = 0; i < ARRAY_SIZE(peripherals); i++) {
    if (peripherals[i].conn == conn) {
      *slot = &peripherals[i];
      goto exit_get_slot_by_connection;
    }
  }

  res = -ENODEV;
exit_get_slot_by_connection:
  k_mutex_unlock(&slot_mutex);
  return res;
}

void release_peripheral_slot(struct peripheral_slot *slot) {
  k_mutex_lock(&slot_mutex, K_FOREVER);

  if (slot->conn != NULL) {
    bt_conn_unref(slot->conn);
    slot->conn = NULL;
  }
  slot->occupied = false;

  for (int i = 0; i < POSITION_STATE_DATA_LEN; i++) {
    slot->position_state[i] = 0U;
    slot->changed_positions[i] = 0U;
  }

  // Clean up previously discovered handles;
  slot->subscribe_params.value_handle = 0;
  slot->run_behavior_handle = 0;

  k_mutex_unlock(&slot_mutex);
}
