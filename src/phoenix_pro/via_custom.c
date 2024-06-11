#include "via.h"
#include "zmk/split/bluetooth/central.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(via_custom, 4);

static void set_pair_addr(uint8_t *data, uint8_t length) {
  switch (data[2]) {
  case 0:
    clear_bond();
    break;
  case 1:
  case 2:
    set_peripheral_addr(data + 3, data[2] - 1);
    break;
  case 3:
  case 4:
    clear_peripheral(data[2] - 3);
    break;
  case 5:
    clear_bond_secrets();
  default:
    LOG_WRN("unsupported set addr command: 0x%x", data[2]);
  }
}

static void custom_set_value(uint8_t *data, uint8_t length) {
  switch (data[1]) {
  case 0:
    set_pair_addr(data, length);
    break;
  default:
    LOG_WRN("unsupported via custom set command: 0x%x", data[1]);
  }
}

static void custom_get_value(uint8_t *data, uint8_t length) {
  switch (data[1]) {
  case 0:
  case 1:
    get_peripheral_addr(data + 2, data[1]);
    break;
  default:
    LOG_WRN("unsupported via custom get command: 0x%x", data[1]);
  }
}

void via_custom_value_command(uint8_t *data, uint8_t length) {
  LOG_HEXDUMP_DBG(data, length, "via custom value command");
  switch (data[0]) {
  case id_lighting_set_value:
    custom_set_value(data, length);
    break;
  case id_lighting_get_value:
    custom_get_value(data, length);
    break;
  default:
    LOG_WRN("unsupported command in custom via: 0x%x", data[0]);
    break;
  }
}
