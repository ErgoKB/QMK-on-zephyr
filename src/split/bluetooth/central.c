#include "zmk/split/bluetooth/connection.h"

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
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

  k_work_queue_start(&split_central_split_run_q,
                     split_central_split_run_q_stack,
                     K_THREAD_STACK_SIZEOF(split_central_split_run_q_stack),
                     CONFIG_ZMK_BLE_THREAD_PRIORITY, NULL);
  bt_conn_cb_register(&conn_callbacks);

  start_scan();
  return 0;
}

SYS_INIT(ble_init, APPLICATION, CONFIG_BLE_INIT_PRIORITY);
