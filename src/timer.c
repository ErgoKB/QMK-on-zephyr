#include <zephyr/kernel.h>

void timer_init(void) {}

uint16_t timer_read(void) { return (uint16_t)k_uptime_get_32(); }

uint32_t timer_read32(void) { return k_uptime_get_32(); }
