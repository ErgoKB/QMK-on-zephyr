#include "timer.h"

#include <zephyr/kernel.h>

void timer_init(void) {}

uint16_t timer_read(void) { return (uint16_t)k_uptime_get_32(); }

uint32_t timer_read32(void) { return k_uptime_get_32(); }

uint16_t timer_elapsed(uint16_t last) {
  return TIMER_DIFF_16(timer_read(), last);
};

uint32_t timer_elapsed32(uint32_t last) {
  return TIMER_DIFF_32(timer_read32(), last);
};
