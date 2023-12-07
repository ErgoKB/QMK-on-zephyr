#pragma once

#include "host_driver.h"

extern host_driver_t zephyr_driver;

void init_usb_driver(void);

#if CONFIG_RAW_ENABLE
void raw_hid_task(void);
#endif /* CONFIG_RAW_ENABLE */
