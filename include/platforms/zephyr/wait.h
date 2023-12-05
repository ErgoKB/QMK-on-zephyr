#pragma once

#include <zephyr/kernel.h>

#define wait_ms(ms) k_msleep(ms)
#define wait_us(us) k_usleep(us)
// TODO (lschyi): improve waitInputPinDelay
#define waitInputPinDelay() wait_us(5)
