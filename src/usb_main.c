#include "usb_main.h"
#include "hid.h"
#if CONFIG_RAW_ENABLE
#include "raw_hid.h"
#endif /* CONFIG_RAW_ENABLE */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/usb_device.h>
LOG_MODULE_DECLARE(qmk_on_zephyr);

#define USB_SEND_INTERVAL K_MSEC(10)

typedef union {
  report_keyboard_t keyboard_report;
  report_mouse_t mouse_report;
#if CONFIG_EXTRAKEY_ENABLE
  report_extra_t consume_report;
#endif /* CONFIG_EXTRAKEY_ENABLE */
} hid_report;

enum Report { Keyboard, Mouse, Consumer };

typedef struct {
  enum Report type;
  hid_report report;
} hid_queue_item;

K_MSGQ_DEFINE(hid_report_queue, sizeof(hid_queue_item), 50, 1);

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
  default:
    return hid_int_ep_write(hid_dev, report, len, NULL);
  }
}

static void send_keyboard(report_keyboard_t *report) {
  // TODO (lschyi) handle boot protocol, do not send ep and other things
  report->report_id = 1;
  hid_queue_item item = {.type = Keyboard};
  memcpy(&(item.report), report, sizeof(report_keyboard_t));
  k_msgq_put(&hid_report_queue, &item, K_NO_WAIT);
};

static void send_mouse(report_mouse_t *report) {
  report->report_id = 2;
  k_sem_take(&hid_sem, USB_SEND_INTERVAL);
  int err = send_report((uint8_t *)report, sizeof(report_mouse_t));
  if (err) {
    k_sem_give(&hid_sem);
  }
}

static void send_extra(report_extra_t *report) {
#if CONFIG_EXTRAKEY_ENABLE
  hid_queue_item item = {.type = Consumer};
  memcpy(&(item.report), report, sizeof(report_extra_t));
  k_msgq_put(&hid_report_queue, &item, K_NO_WAIT);
#endif /* CONFIG_EXTRAKEY_ENABLE */
}

host_driver_t zephyr_driver = {
    .keyboard_leds = keyboard_leds,
    .send_keyboard = send_keyboard,
    .send_mouse = send_mouse,
    .send_extra = send_extra,
};

#if CONFIG_RAW_ENABLE
void raw_hid_send(uint8_t *data, uint8_t length) {
  int err = hid_int_ep_write(via_dev, data, length, NULL);
  if (err) {
    LOG_ERR("Failed to write data via raw hid endpoint, err = %d", err);
  }
}

void raw_hid_task(void) {
  static uint8_t buffer[RAW_EPSIZE];
  static uint8_t idx = 0;
  uint32_t size = 0;
  int ret;

  if (!via_out_ready) {
    return;
  }

  do {
    ret = hid_int_ep_read(via_dev, buffer + idx, RAW_EPSIZE, &size);
    if (ret != 0) {
      LOG_ERR("Encounter error while receiving data from raw HID endpoint: %d",
              ret);
      break;
    }
    if (size > 0) {
      idx += size;
      if (idx > RAW_EPSIZE) {
        LOG_ERR("Got RAW HID transaction more thant RAW_EPSIZE(%d) bytes",
                RAW_EPSIZE);
        idx = 0;
        return;
      }
      if (idx == RAW_EPSIZE) {
        raw_hid_receive(buffer, sizeof(buffer));
        idx = 0;
      }
    }
  } while (size > 0);
}
#endif /* CONFIG_RAW_ENABLE */

static void send_report_with_retry(uint8_t *report, size_t len, uint8_t retry) {
  for (uint8_t i = 0; i < retry; i++) {
    if (send_report(report, len) == 0) {
      return;
    }
    k_sleep(USB_SEND_INTERVAL);
  }
}

static void hid_send_worker(void) {
  hid_queue_item item;
  while (true) {
    k_msgq_get(&hid_report_queue, &item, K_FOREVER);
    switch (item.type) {
    case Keyboard:
      send_report_with_retry((uint8_t *)&(item.report.keyboard_report.report_id),
                  KEYBOARD_REPORT_SIZE, 5);
      break;
#if CONFIG_EXTRAKEY_ENABLE
    case Consumer:
      send_report_with_retry((uint8_t *)&(item.report.consume_report),
                  sizeof(report_extra_t), 5);
      break;
#endif /* CONFIG_EXTRAKEY_ENABLE */
    default:
      break;
    }
    // sleep for a small period of time to send next HID package  for some host
    // os will do debounce internally.
    k_sleep(USB_SEND_INTERVAL);
  }
}

K_THREAD_DEFINE(hid_send, 1024, hid_send_worker, NULL, NULL, NULL, 7, 0, 0);
