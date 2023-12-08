#include "usb_main.h"
#include "hid.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/usb_device.h>
LOG_MODULE_DECLARE(qmk_on_zephyr);

static K_SEM_DEFINE(hid_sem, 1, 1);

static enum usb_dc_status_code usb_status = USB_DC_UNKNOWN;

static const struct device *hid_dev;
static void in_ready_cb(const struct device *dev) { k_sem_give(&hid_sem); }

static const struct hid_ops hid_dev_ops = {
    .int_in_ready = in_ready_cb,
};

static bool via_out_ready = false;
static const struct device *via_dev;
static void via_out_ready_cb(const struct device *dev) { via_out_ready = true; }
static const struct hid_ops via_dev_ops = {
    .int_out_ready = via_out_ready_cb,
};

static void usb_status_cb(enum usb_dc_status_code status,
                          const uint8_t *params) {
  usb_status = status;
}

void init_usb_driver(void) {
  int ret;

  hid_dev = device_get_binding("HID_0");
  if (hid_dev == NULL) {
    LOG_ERR("Cannot get USB HID Device");
    return;
  }
  usb_hid_register_device(hid_dev, hid_kbd_report_desc,
                          sizeof(hid_kbd_report_desc), &hid_dev_ops);
  usb_hid_init(hid_dev);

  via_dev = device_get_binding("HID_1");
  if (via_dev == NULL) {
    LOG_ERR("Cannot get RAW HID Device");
    return;
  }
  usb_hid_register_device(via_dev, via_hid_report_desc,
                          sizeof(via_hid_report_desc), &via_dev_ops);
  usb_hid_init(via_dev);

  ret = usb_enable(usb_status_cb);
  if (ret) {
    LOG_ERR("Failed to enable USB");
    return;
  }
}

static uint8_t keyboard_leds(void) { return 0; };

static int send_report(uint8_t *report, size_t len) {
  switch (usb_status) {
  case USB_DC_SUSPEND:
    return usb_wakeup_request();
  case USB_DC_ERROR:
  case USB_DC_RESET:
  case USB_DC_DISCONNECTED:
  case USB_DC_UNKNOWN:
    return -ENODEV;
  default:
    k_sem_take(&hid_sem, K_MSEC(30));
    int err = hid_int_ep_write(hid_dev, report, len, NULL);

    if (err) {
      k_sem_give(&hid_sem);
    }

    return err;
  }
}

static void send_keyboard(report_keyboard_t *report) {
  // TODO (lschyi) handle boot protocol, do not send ep and other things
  report->report_id = 1;
  send_report((uint8_t *)&(report->report_id), KEYBOARD_REPORT_SIZE);
};

host_driver_t zephyr_driver = {
    .keyboard_leds = keyboard_leds,
    .send_keyboard = send_keyboard,
};

#if CONFIG_RAW_ENABLE
void raw_hid_task(void) {
  uint8_t buffer[RAW_EPSIZE];
  uint32_t size = 0;
  int ret;

  if (!via_out_ready) {
    return;
  }

  do {
    ret = hid_int_ep_read(via_dev, buffer, RAW_EPSIZE, &size);
    if (ret != 0) {
      LOG_ERR("Encounter error while receiving data from raw HID endpoint: %d",
              ret);
      break;
    }
    if (size > 0) {
      // TODO (lschyi): call via commands
      LOG_INF("Get data");
    }
  } while (size > 0);
}
#endif /* CONFIG_RAW_ENABLE */