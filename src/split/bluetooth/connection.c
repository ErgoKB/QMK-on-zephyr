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

static bool is_scanning = false;
static bool is_provisioning = true;

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad);
static bool parse_device_data(struct bt_data *data, void *user_data);
static bool try_connect_split_device(const bt_addr_le_t *addr,
                                     struct peripheral_slot *slot);
static int split_central_process_connection(struct bt_conn *conn,
                                            struct peripheral_slot *slot);
static uint8_t
split_central_service_discovery_func(struct bt_conn *conn,
                                     const struct bt_gatt_attr *attr,
                                     struct bt_gatt_discover_params *params);
static uint8_t
split_central_chrc_discovery_func(struct bt_conn *conn,
                                  const struct bt_gatt_attr *attr,
                                  struct bt_gatt_discover_params *params);

static uint8_t split_central_chrc_discovery_func_wrapper(
    struct bt_conn *conn, const struct bt_gatt_attr *attr,
    struct bt_gatt_discover_params *params);

static uint8_t
split_central_notify_func(struct bt_conn *conn,
                          struct bt_gatt_subscribe_params *params,
                          const void *data, uint16_t length);

void set_is_provisioning(bool val) { is_provisioning = val; }

void security_changed(struct bt_conn *conn, bt_security_t level,
                      enum bt_security_err err) {
  if (!err) {
    LOG_INF("Changed security success");
    struct peripheral_slot *slot = get_slot_by_connection(conn);
    if (split_central_process_connection(conn, slot)) {
      // trigger a disconnect to also trigger start scan
      bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }
    return;
  }
  LOG_ERR("Change security failed, unpair");
  bt_unpair(BT_ID_DEFAULT, bt_conn_get_dst(conn));
}

bool split_central_connected_work(struct bt_conn *conn, uint8_t conn_err) {
  char addr[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  if (conn_err) {
    LOG_ERR("Failed to connect to %s (%u)", addr, conn_err);
    return false;
  }

  LOG_DBG("Connected: %s", addr);

  struct bt_conn_info conn_info;
  if (bt_conn_get_info(conn, &conn_info)) {
    LOG_ERR("Failed to get connection info");
    return false;
  }

  if (conn_info.role == BT_CONN_ROLE_CENTRAL) {
    LOG_INF("Role is central, setting security");
    int err = bt_conn_set_security(conn, BT_SECURITY_L2);
    if (err) {
      LOG_ERR("Failed to set security; %d", err);
      return false;
    }
  }

  struct peripheral_slot *slot = get_slot_by_connection(conn);
  if (slot == NULL) {
    LOG_ERR("Cannot find corresponding slot");
    return false;
  }

  confirm_peripheral_slot_conn(conn);
  return true;
}

void split_central_disconnected(struct bt_conn *conn, uint8_t reason) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  LOG_DBG("Disconnected: %s (reason %d)", addr, reason);
  release_peripheral_slot_for_conn(conn);
  bt_unpair(BT_ID_DEFAULT, bt_conn_get_dst(conn));

  start_scan();
}

void split_central_connected(struct bt_conn *conn, uint8_t conn_err) {
  if (!split_central_connected_work(conn, conn_err)) {
    release_peripheral_slot_for_conn(conn);
  }
}

void start_scan(void) {
  if (is_scanning) {
    LOG_DBG("Already scanning, skip");
    return;
  }

  if (!has_open_slot()) {
    LOG_DBG("No empty slot, no need to scan");
    return;
  }
  uint32_t options = BT_LE_SCAN_OPT_FILTER_DUPLICATE;
  if (!is_provisioning) {
    options |= BT_LE_SCAN_OPT_FILTER_ACCEPT_LIST;
  }
  int err = bt_le_scan_start(BT_LE_SCAN_PARAM(BT_LE_SCAN_TYPE_PASSIVE, options,
                                              BT_GAP_SCAN_FAST_INTERVAL,
                                              BT_GAP_SCAN_FAST_WINDOW),
                             device_found);
  if (err) {
    LOG_ERR("Scanning failed to start (err %d)", err);
    return;
  }
  is_scanning = true;
  LOG_DBG("Scanning successfully");
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad) {
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
  bool is_split_service = false;
  bt_data_parse(ad, parse_device_data, (void *)&is_split_service);
  if (!is_split_service) {
    return;
  }

  int err = bt_le_scan_stop();
  if (err) {
    LOG_ERR("Stop LE scan failed (err %d)", err);
    return;
  }
  is_scanning = false;

  int slot_idx = reserve_peripheral_slot(addr);
  if (slot_idx < 0) {
    LOG_DBG("Failed to reserve slot");
    start_scan();
    return;
  }

  struct peripheral_slot *slot = &peripherals[slot_idx];
  if (!try_connect_split_device(addr, slot)) {
    LOG_DBG("Failed to connect to slot");
    release_peripheral_slot(slot_idx);
    start_scan();
  }
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
  slot->conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, addr);
  if (slot->conn) {
    LOG_DBG("Found existing connection");
    return true;
  }
  struct bt_le_conn_param *param = BT_LE_CONN_PARAM(0x0006, 0x0006, 0, 40);

  LOG_DBG("Initiating new connnection");

  int err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param, &slot->conn);
  if (err) {
    LOG_ERR("Create conn failed (err %d) (create conn? 0x%04x)", err,
            BT_HCI_OP_LE_CREATE_CONN);
    return false;
  }
  return true;
}

static int split_central_process_connection(struct bt_conn *conn,
                                            struct peripheral_slot *slot) {
  slot->discover_params.uuid = &split_service_uuid.uuid;
  slot->discover_params.func = split_central_service_discovery_func;
  slot->discover_params.start_handle = 0x0001;
  slot->discover_params.end_handle = 0xffff;
  slot->discover_params.type = BT_GATT_DISCOVER_PRIMARY;

  int err = bt_gatt_discover(slot->conn, &slot->discover_params);
  if (err) {
    LOG_ERR("Discover failed(err %d)", err);
    return err;
  }
  return 0;
}

static uint8_t
split_central_notify_func(struct bt_conn *conn,
                          struct bt_gatt_subscribe_params *params,
                          const void *data, uint16_t length) {
  struct peripheral_slot *slot = get_slot_by_connection(conn);

  if (slot == NULL) {
    LOG_ERR("No peripheral state found for connection");
    return BT_GATT_ITER_CONTINUE;
  }

  if (!data) {
    LOG_DBG("[UNSUBSCRIBED]");
    params->value_handle = 0U;
    return BT_GATT_ITER_STOP;
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

  struct peripheral_slot *slot = get_slot_by_connection(conn);
  if (slot == NULL) {
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

static void split_central_subscribe(struct bt_conn *conn) {
  struct peripheral_slot *slot = get_slot_by_connection(conn);
  if (slot == NULL) {
    LOG_ERR("No peripheral state found for connection");
    return;
  }

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

static uint8_t
split_central_chrc_discovery_func(struct bt_conn *conn,
                                  const struct bt_gatt_attr *attr,
                                  struct bt_gatt_discover_params *params) {
  if (!attr) {
    LOG_DBG("Discover complete");
    return BT_GATT_ITER_STOP;
  }

  if (!attr->user_data) {
    LOG_ERR("Required user data not passed to discovery");
    return BT_GATT_ITER_STOP;
  }

  struct peripheral_slot *slot = get_slot_by_connection(conn);
  if (slot == NULL) {
    LOG_ERR("No peripheral state found for connection");
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
    split_central_subscribe(conn);
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
  uint8_t res = split_central_chrc_discovery_func(conn, attr, params);
  if (res == BT_GATT_ITER_STOP) {
    struct peripheral_slot *slot = get_slot_by_connection(conn);
    bool subscribed =
        (slot->run_behavior_handle && slot->subscribe_params.value_handle);
    if (!subscribed) {
      bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    } else {
      start_scan();
    }
  }
  return res;
}
