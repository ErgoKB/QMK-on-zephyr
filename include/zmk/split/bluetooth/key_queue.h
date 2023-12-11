#pragma once

#include <zephyr/kernel.h>

struct key_event {
  bool pressed;
  int position;
};

extern struct k_msgq key_queue;
