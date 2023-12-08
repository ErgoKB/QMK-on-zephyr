#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/bluetooth/conn.h>

void set_is_provisioning(bool);
void start_scan(void);
void split_central_connected(struct bt_conn *conn, uint8_t conn_err);
void split_central_disconnected(struct bt_conn *conn, uint8_t reason);
void security_changed(struct bt_conn *conn, bt_security_t level,
                      enum bt_security_err err);
