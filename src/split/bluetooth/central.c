#include "zmk/split/bluetooth/central.h"
#include "zmk/split/bluetooth/connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_SETTINGS)
#include <zephyr/settings/settings.h>
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

LOG_MODULE_REGISTER(ble_central, 4);

K_THREAD_STACK_DEFINE(split_central_split_run_q_stack,
                      CONFIG_ZMK_BLE_SPLIT_CENTRAL_SPLIT_RUN_STACK_SIZE);
struct k_work_q split_central_split_run_q;

static struct bt_conn_cb conn_callbacks = {
    .connected = split_central_connected,
    .disconnected = split_central_disconnected,
    .security_changed = security_changed,
};

#if IS_ENABLED(CONFIG_SETTINGS)
static inline bool can_addr_filter() {
  return recorded_peripherals == ARRAY_SIZE(peripheral_addrs);
}

static int ble_profiles_handler_set(const char *name, size_t len,
                                    settings_read_cb read_cb, void *cb_arg) {
  const char *next;
  if (settings_name_steq(name, "peripheral_addresses", &next) && next) {
    int i = atoi(next);
    if (i >= ARRAY_SIZE(peripheral_addrs)) {
      LOG_ERR("load settings error: peripheral slot out of range");
      return -EINVAL;
    }

    if (len != sizeof(bt_addr_le_t)) {
      LOG_ERR("Stored peripheral idx %d address size is invalid", i);
      return -EINVAL;
    }

    int err = read_cb(cb_arg, &peripheral_addrs[i], sizeof(bt_addr_le_t));
    if (err <= 0) {
      LOG_ERR("Failed to handle peripheral address from settings: %d", err);
      return err;
    }
    ++recorded_peripherals;
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(&peripheral_addrs[i], addr_str, sizeof(addr_str));
    LOG_DBG("Reading peripheral %s from idx %i", addr_str, i);
  }
  return 0;
}

struct settings_handler profiles_handler = {.name = "ble",
                                            .h_set = ble_profiles_handler_set};
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

static int ble_init(const struct device *_arg) {
  int err = bt_enable(NULL);
  if (err) {
    LOG_ERR("Bluetooth init failed: %d", err);
    return err;
  }

#if IS_ENABLED(CONFIG_SETTINGS)
  err = settings_subsys_init();
  if (err) {
    LOG_ERR("Failed to init subsystem: %d", err);
  }
  err = settings_load();
  if (err) {
    LOG_ERR("Failed to load setting: %d", err);
  }
  err = settings_register(&profiles_handler);
  if (err) {
    LOG_ERR("Failed to register profile handler: %d", err);
  }
  err = settings_load_subtree("ble");
  if (err) {
    LOG_ERR("Failed to load ble settings: %d", err);
  }
  if (recorded_peripherals == ARRAY_SIZE(peripheral_addrs)) {
    for (int i = 0; i < ARRAY_SIZE(peripheral_addrs); i++) {
      int err = bt_le_filter_accept_list_add(&peripheral_addrs[i]);
      if (err) {
        LOG_ERR("Failed to add %dth addr to filter: %d", i, err);
      }
    }
  }
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

  k_work_queue_start(&split_central_split_run_q,
                     split_central_split_run_q_stack,
                     K_THREAD_STACK_SIZEOF(split_central_split_run_q_stack),
                     CONFIG_ZMK_BLE_THREAD_PRIORITY, NULL);
  bt_conn_cb_register(&conn_callbacks);

  start_scan();
  return 0;
}

void clear_bond() {
#if IS_ENABLED(CONFIG_SETTINGS)
  LOG_DBG("Clear bonding information");
  char setting_name[32];
  for (int i = 0; i < ARRAY_SIZE(peripheral_addrs); i++) {
    sprintf(setting_name, "ble/peripheral_addresses/%d", i);
    settings_delete(setting_name);
  }
  bt_unpair(BT_ID_DEFAULT, NULL);
#endif /* IS_ENABLED(CONFIG_SETTINGS) */
}

void clear_bond_secrets() {
#if IS_ENABLED(CONFIG_SETTINGS)
  LOG_DBG("Clear bonding secrets");
  bt_unpair(BT_ID_DEFAULT, NULL);
#endif /* IS_ENABLED(CONFIG_SETTINGS) */
}

void set_peripheral_addr(uint8_t *data, int idx) {
#if IS_ENABLED(CONFIG_SETTINGS)
  bt_addr_le_t addr = {
      .type = BT_ADDR_LE_RANDOM,
      .a =
          {
              .val = {data[0], data[1], data[2], data[3], data[4], data[5]},
          },
  };
  char setting_name[32];
  sprintf(setting_name, "ble/peripheral_addresses/%d", idx);
  settings_save_one(setting_name, &addr, sizeof(bt_addr_le_t));
#endif /* IS_ENABLED(CONFIG_SETTINGS) */
}

void clear_peripheral(int idx) {
  if (idx >= ARRAY_SIZE(peripheral_addrs)) {
    return;
  }
  char setting_name[32];
  sprintf(setting_name, "ble/peripheral_addresses/%d", idx);
  settings_delete(setting_name);
  bt_unpair(BT_ID_DEFAULT, &peripheral_addrs[idx]);
}

void get_peripheral_addr(uint8_t *data, int idx) {
  if (idx >= ARRAY_SIZE(peripheral_addrs)) {
    return;
  }
  for (int i = 0; i < 6; i++) {
    data[i] = peripheral_addrs[idx].a.val[i];
  }
}

SYS_INIT(ble_init, APPLICATION, CONFIG_BLE_INIT_PRIORITY);
