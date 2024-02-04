#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/bluetooth/conn.h>

#if IS_ENABLED(CONFIG_SETTINGS)
extern uint8_t recorded_peripherals;
extern bt_addr_le_t peripheral_addrs[CONFIG_ZMK_BLE_SPLIT_PERIPHERAL_COUNT];
#endif /* IS_ENABLED(CONFIG_SETTINGS) */
void set_is_provisioning(bool);
void start_scan(void);
void split_central_connected(struct bt_conn *conn, uint8_t conn_err);
void split_central_disconnected(struct bt_conn *conn, uint8_t reason);
void security_changed(struct bt_conn *conn, bt_security_t level,
                      enum bt_security_err err);
