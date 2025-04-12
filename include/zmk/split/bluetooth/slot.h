#pragma once

#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>

#define POSITION_STATE_DATA_LEN 16
#define ZMK_BLE_SPLIT_PERIPHERAL_COUNT CONFIG_ZMK_BLE_SPLIT_PERIPHERAL_COUNT

struct peripheral_slot {
  bool occupied;
  struct bt_conn *conn;
  struct bt_gatt_discover_params discover_params;
  struct bt_gatt_subscribe_params subscribe_params;
  struct bt_gatt_discover_params sub_discover_params;
  uint16_t run_behavior_handle;
  uint8_t position_state[POSITION_STATE_DATA_LEN];
  uint8_t changed_positions[POSITION_STATE_DATA_LEN];
};

bool has_open_slot(void);
int request_peripheral_slot(const bt_addr_le_t *addr,
                            struct peripheral_slot **slot);
int get_slot_by_connection(struct bt_conn *conn, struct peripheral_slot **slot);
void release_peripheral_slot(struct peripheral_slot *slot);
