#include "zmk/split/bluetooth/connection.h"
#include "zmk/split/bluetooth/key_queue.h"
#include "zmk/split/bluetooth/slot.h"
#include "zmk/split/bluetooth/uuid.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(ble_central, 3);

K_MSGQ_DEFINE(key_queue, sizeof(struct key_event), 30, 1);

static const struct bt_uuid_128 split_service_uuid =
    BT_UUID_INIT_128(ZMK_SPLIT_BT_SERVICE_UUID);

#if IS_ENABLED(CONFIG_SETTINGS)
uint8_t recorded_peripherals = 0;
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

static void find_split_device(const bt_addr_le_t *addr, int8_t rssi,
                              uint8_t type, struct net_buf_simple *ad);
static bool parse_device_data(struct bt_data *data, void *user_data);
static bool try_connect_split_device(const bt_addr_le_t *addr,
                                     struct peripheral_slot *slot);
static uint8_t
split_central_service_discovery_func(struct bt_conn *conn,
                                     const struct bt_gatt_attr *attr,
                                     struct bt_gatt_discover_params *params);
static uint8_t split_central_chrc_discovery_func(
    struct bt_conn *conn, const struct bt_gatt_attr *attr,
    struct bt_gatt_discover_params *params, struct peripheral_slot *slot);

static uint8_t split_central_chrc_discovery_func_wrapper(
    struct bt_conn *conn, const struct bt_gatt_attr *attr,
    struct bt_gatt_discover_params *params);

static uint8_t
split_central_notify_func(struct bt_conn *conn,
                          struct bt_gatt_subscribe_params *params,
                          const void *data, uint16_t length);

/*
 * split_central_disconnected performs device disconnected clean up, and
 * **triggers** running scanning (since there must be new empty slot).
 * The slot is released in this call.
 * This call shall not fail (or just fail with ignorable error).
 */
void split_central_disconnected(struct bt_conn *conn, uint8_t reason) {
  char addr[BT_ADDR_LE_STR_LEN];
  struct peripheral_slot *slot;
  uint32_t position;
  struct key_event ev;

  // No slot found, this connection is not in our split processing flow.
  if (get_slot_by_connection(conn, &slot)) {
    return;
  }

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  LOG_DBG("Disconnected: %s (reason %d)", addr, reason);

  for (int i = 0; i < POSITION_STATE_DATA_LEN; i++) {
    for (int j = 0; j < 8; j++) {
      if (slot->position_state[i] & BIT(j)) {
        position = (i * 8) + j;
        ev.position = position;
        ev.pressed = false;
        k_msgq_put(&key_queue, &ev, K_NO_WAIT);
      }
    }
  }

  release_peripheral_slot(slot);
#if IS_ENABLED(CONFIG_SETTINGS)
  if (recorded_peripherals != CONFIG_ZMK_BLE_SPLIT_PERIPHERAL_COUNT) {
    bt_unpair(BT_ID_DEFAULT, bt_conn_get_dst(conn));
  }
#else
  bt_unpair(BT_ID_DEFAULT, bt_conn_get_dst(conn));
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

  start_scan();
}

/*
 * split_central_connected just reserves a slot for the connection. The
 * connection need to go through the security change process. Only if security
 * change process passed, we then consider the device is online and is working.
 *
 * For stability, we will only start the scanning of the next device until the
 * current complete process is completed. If any error encountered during the
 * process, the device is considered offline, and corresponding resources should
 * be released and restored.
 *
 * As such, the only thing this callback should do is just initiate the security
 * change call.
 */
void split_central_connected(struct bt_conn *conn, uint8_t conn_err) {
  char addr[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  struct peripheral_slot *slot;

  // No slot found, this connection is not in our split processing flow.
  if (get_slot_by_connection(conn, &slot)) {
    return;
  }

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  /*
   * The connection slot is reserved during the scanning call, so when
   * encountering any error in this callback, the slot must be released.
   */
  if (conn_err) {
    LOG_ERR("Failed to connect to %s (%u)", addr, conn_err);
    goto connected_drop_this_connection;
  }

  LOG_DBG("Connected: %s", addr);

  int err = bt_conn_set_security(conn, BT_SECURITY_L2);
  if (err) {
    LOG_ERR("Failed to set security; %d", err);
    goto connected_drop_this_connection;
  }

  return;

connected_drop_this_connection:
  bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

/*
 * security_changed is the callback that after bonding is processed. In our
 * split connection flow, it is not completed yet. Only until the GATT discovery
 * is complete, we can make sure the connected device contains the desire GATT
 * service we watn, we then consider the device is online and ready, then we can
 * start a new scanning process.
 *
 * As such, the only thing this callback should do is just trigger the GATT
 * discovery.
 */
void security_changed(struct bt_conn *conn, bt_security_t level,
                      enum bt_security_err err) {
  struct peripheral_slot *slot;
  int ret;

  // No slot found, this connection is not in our split processing flow.
  if (get_slot_by_connection(conn, &slot)) {
    return;
  }

  if (err) {
    LOG_ERR("Change security failed");
    goto security_changed_drop_this_connection;
  }

  LOG_DBG("Changed security success");

  slot->discover_params.uuid = &split_service_uuid.uuid;
  slot->discover_params.func = split_central_service_discovery_func;
  slot->discover_params.start_handle = 0x0001;
  slot->discover_params.end_handle = 0xffff;
  slot->discover_params.type = BT_GATT_DISCOVER_PRIMARY;

  ret = bt_gatt_discover(slot->conn, &slot->discover_params);
  if (ret) {
    LOG_ERR("Failed to perform GATT discovery: %d", err);
    goto security_changed_drop_this_connection;
  }
  return;

security_changed_drop_this_connection:
  bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

void start_scan(void) {
  uint32_t options = BT_LE_SCAN_OPT_FILTER_DUPLICATE;
  int err;

  if (!has_open_slot()) {
    LOG_DBG("No empty slot, no need to scan");
    return;
  }

#if IS_ENABLED(CONFIG_SETTINGS)
  if (recorded_peripherals == CONFIG_ZMK_BLE_SPLIT_PERIPHERAL_COUNT) {
    LOG_INF("All peripherals slot are recorded, apply filter");
    options |= BT_LE_SCAN_OPT_FILTER_ACCEPT_LIST;
  } else {
    LOG_INF("Not all slot are recorded, do not apply filter");
  }
#endif /* IS_ENABLED(CONFIG_SETTINGS) */

  err = bt_le_scan_start(BT_LE_SCAN_PARAM(BT_LE_SCAN_TYPE_PASSIVE, options,
                                          BT_GAP_SCAN_FAST_INTERVAL,
                                          BT_GAP_SCAN_FAST_WINDOW),
                         find_split_device);
  if (err && err != -EBUSY) {
    LOG_ERR("Scanning failed to start (err %d)", err);
  }
  LOG_DBG("Scanning successfully");
}

static void find_split_device(const bt_addr_le_t *addr, int8_t rssi,
                              uint8_t type, struct net_buf_simple *ad) {
  bool is_split_service = false;
  int err;
  struct peripheral_slot *slot;

  if ((type != BT_GAP_ADV_TYPE_ADV_IND &&
       type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND)) {
    return;
  }

  /* TODO (lschyi): this is a boost
  const bt_addr_le_t *peripheral_addrs = zmk_ble_get_peripheral_addrs();
  for (int i = 0; i < ZMK_BLE_SPLIT_PERIPHERAL_COUNT; i++) {
    if (bt_addr_le_cmp(&peripheral_addrs[i], addr) == 0) {
      LOG_INF("found remembered device, try connect it without checking");
      split_central_eir_found(addr);
      return;
    }
  }
  */
  bt_data_parse(ad, parse_device_data, (void *)&is_split_service);
  if (!is_split_service) {
    return;
  }

  /*
   * We found a possible split device, now stop scanning, reserve the radio
   * until we complete the processing with this split device.
   */
  err = bt_le_scan_stop();
  if (err) {
    LOG_ERR("Stop LE scan failed (err %d)", err);
    return;
  }

  err = request_peripheral_slot(addr, &slot);
  if (err) {
    LOG_DBG("Failed to reserve slot");
    goto device_found_revert_scanning;
  }

  if (!try_connect_split_device(addr, slot)) {
    LOG_DBG("Failed to connect to slot");
    release_peripheral_slot(slot);
    goto device_found_revert_scanning;
  }
  return;

device_found_revert_scanning:
  start_scan();
}

static bool parse_device_data(struct bt_data *data, void *user_data) {
  bool *is_split_service = user_data;

  switch (data->type) {
  case BT_DATA_UUID128_SOME:
  case BT_DATA_UUID128_ALL:
    if (data->data_len % 16 != 0U) {
      LOG_ERR("AD malformed");
      return true;
    }

    for (int i = 0; i < data->data_len; i += 16) {
      struct bt_uuid_128 uuid;

      if (!bt_uuid_create(&uuid.uuid, &data->data[i], 16)) {
        LOG_ERR("Unable to load UUID");
        continue;
      }

      if (bt_uuid_cmp(&uuid.uuid,
                      BT_UUID_DECLARE_128(ZMK_SPLIT_BT_SERVICE_UUID))) {
        continue;
      }

      *is_split_service = true;
      return false;
    }
  }

  return true;
}

static bool try_connect_split_device(const bt_addr_le_t *addr,
                                     struct peripheral_slot *slot) {
  struct bt_le_conn_param *param = BT_LE_CONN_PARAM(0x0006, 0x0006, 0, 40);

  LOG_DBG("Initiating new connection");

  int err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param, &slot->conn);
  if (err) {
    LOG_ERR("Create conn failed (err %d) (create conn? 0x%04x)", err,
            BT_HCI_OP_LE_CREATE_CONN);
    return false;
  }
  return true;
}

static uint8_t
split_central_notify_func(struct bt_conn *conn,
                          struct bt_gatt_subscribe_params *params,
                          const void *data, uint16_t length) {
  struct peripheral_slot *slot;

  if (get_slot_by_connection(conn, &slot)) {
    LOG_ERR("No peripheral state found for connection");
    return BT_GATT_ITER_STOP;
  }

  if (!data) {
    LOG_DBG("[UNSUBSCRIBED]");
    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    return BT_GATT_ITER_CONTINUE;
  }

  LOG_DBG("[NOTIFICATION] data %p length %u", data, length);

  for (int i = 0; i < POSITION_STATE_DATA_LEN; i++) {
    slot->changed_positions[i] = ((uint8_t *)data)[i] ^ slot->position_state[i];
    slot->position_state[i] = ((uint8_t *)data)[i];
    LOG_DBG("data: %d", slot->position_state[i]);
  }

  for (int i = 0; i < POSITION_STATE_DATA_LEN; i++) {
    for (int j = 0; j < 8; j++) {
      if (slot->changed_positions[i] & BIT(j)) {
        uint32_t position = (i * 8) + j;
        bool pressed = slot->position_state[i] & BIT(j);
        struct key_event ev = {
            .position = position,
            .pressed = pressed,
        };

        k_msgq_put(&key_queue, &ev, K_NO_WAIT);
      }
    }
  }

  return BT_GATT_ITER_CONTINUE;
}

static uint8_t
split_central_service_discovery_func(struct bt_conn *conn,
                                     const struct bt_gatt_attr *attr,
                                     struct bt_gatt_discover_params *params) {
  if (!attr) {
    LOG_DBG("Discover complete");
    (void)memset(params, 0, sizeof(*params));
    return BT_GATT_ITER_STOP;
  }

  LOG_DBG("[ATTRIBUTE] handle %u", attr->handle);

  struct peripheral_slot *slot;
  if (get_slot_by_connection(conn, &slot)) {
    LOG_ERR("No peripheral state found for connection");
    return BT_GATT_ITER_STOP;
  }

  if (bt_uuid_cmp(slot->discover_params.uuid,
                  BT_UUID_DECLARE_128(ZMK_SPLIT_BT_SERVICE_UUID))) {
    LOG_DBG("Found other service");
    return BT_GATT_ITER_CONTINUE;
  }

  LOG_DBG("Found split service");
  slot->discover_params.uuid = NULL;
  slot->discover_params.func = split_central_chrc_discovery_func_wrapper;
  slot->discover_params.start_handle = attr->handle + 1;
  slot->discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

  int err = bt_gatt_discover(conn, &slot->discover_params);
  if (err) {
    LOG_ERR(
        "Failed to start discovering split service characteristics (err %d)",
        err);
  }
  return BT_GATT_ITER_STOP;
}

static void split_central_subscribe(struct bt_conn *conn,
                                    struct peripheral_slot *slot) {
  int err = bt_gatt_subscribe(conn, &slot->subscribe_params);
  switch (err) {
  case -EALREADY:
    LOG_DBG("[ALREADY SUBSCRIBED]");
    break;
  case 0:
    LOG_DBG("[SUBSCRIBED]");
    break;
  default:
    LOG_ERR("Subscribe failed (err %d)", err);
    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    break;
  }
}

static uint8_t split_central_chrc_discovery_func(
    struct bt_conn *conn, const struct bt_gatt_attr *attr,
    struct bt_gatt_discover_params *params, struct peripheral_slot *slot) {
  if (!attr) {
    LOG_DBG("Discover complete");
    return BT_GATT_ITER_STOP;
  }

  if (!attr->user_data) {
    LOG_ERR("Required user data not passed to discovery");
    return BT_GATT_ITER_STOP;
  }

  LOG_DBG("[ATTRIBUTE] handle %u", attr->handle);

  if (!bt_uuid_cmp(
          ((struct bt_gatt_chrc *)attr->user_data)->uuid,
          BT_UUID_DECLARE_128(ZMK_SPLIT_BT_CHAR_POSITION_STATE_UUID))) {
    LOG_DBG("Found position state characteristic");
    slot->discover_params.uuid = NULL;
    slot->discover_params.start_handle = attr->handle + 2;
    slot->discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

    slot->subscribe_params.disc_params = &slot->sub_discover_params;
    slot->subscribe_params.end_handle = slot->discover_params.end_handle;
    slot->subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);
    slot->subscribe_params.notify = split_central_notify_func;
    slot->subscribe_params.value = BT_GATT_CCC_NOTIFY;
    split_central_subscribe(conn, slot);
  } else if (!bt_uuid_cmp(
                 ((struct bt_gatt_chrc *)attr->user_data)->uuid,
                 BT_UUID_DECLARE_128(ZMK_SPLIT_BT_CHAR_RUN_BEHAVIOR_UUID))) {
    LOG_DBG("Found run behavior handle");
    slot->run_behavior_handle = bt_gatt_attr_value_handle(attr);
  }

  bool subscribed =
      (slot->run_behavior_handle && slot->subscribe_params.value_handle);

  return subscribed ? BT_GATT_ITER_STOP : BT_GATT_ITER_CONTINUE;
}

static uint8_t split_central_chrc_discovery_func_wrapper(
    struct bt_conn *conn, const struct bt_gatt_attr *attr,
    struct bt_gatt_discover_params *params) {
  struct peripheral_slot *slot;
  uint8_t res;

  if (get_slot_by_connection(conn, &slot)) {
    // the slot somehow becomes unavailable, disconnect with this device
    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    return BT_GATT_ITER_STOP;
  }

  res = split_central_chrc_discovery_func(conn, attr, params, slot);
  if (res != BT_GATT_ITER_STOP) {
    return res;
  }

  /*
   * At this point, we have completely iterated the GATT services on this
   * device, so if it is still not subscribed, something went wrong on this
   * discover, and we have to disconnect with this device.
   */
  bool subscribed =
      (slot->run_behavior_handle && slot->subscribe_params.value_handle);
  if (!subscribed) {
    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
  }
  // If there is still empty slots, restart the scanning
  start_scan();
  return BT_GATT_ITER_STOP;
}
