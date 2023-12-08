#pragma once

#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>

#define POSITION_STATE_DATA_LEN 16
#define ZMK_BLE_SPLIT_PERIPHERAL_COUNT CONFIG_ZMK_BLE_SPLIT_PERIPHERAL_COUNT

enum peripheral_slot_state {
  PERIPHERAL_SLOT_STATE_OPEN,
  PERIPHERAL_SLOT_STATE_CONNECTING,
  PERIPHERAL_SLOT_STATE_CONNECTED,
};

struct peripheral_slot {
  enum peripheral_slot_state state;
  struct bt_conn *conn;
  struct bt_gatt_discover_params discover_params;
  struct bt_gatt_subscribe_params subscribe_params;
  struct bt_gatt_discover_params sub_discover_params;
  uint16_t run_behavior_handle;
  uint8_t position_state[POSITION_STATE_DATA_LEN];
  uint8_t changed_positions[POSITION_STATE_DATA_LEN];
};

extern struct peripheral_slot peripherals[ZMK_BLE_SPLIT_PERIPHERAL_COUNT];

bool has_open_slot(void);
int reserve_peripheral_slot(const bt_addr_le_t *addr);
void confirm_peripheral_slot_conn(struct bt_conn *conn);
struct peripheral_slot *get_slot_by_connection(struct bt_conn *conn);
void release_peripheral_slot(int index);
void release_peripheral_slot_for_conn(struct bt_conn *conn);
