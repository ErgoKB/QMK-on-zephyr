#pragma once

#include <stdint.h>
#include "keycodes.h"

#define pgm_read_word(addr) (*(const unsigned short *)(addr))

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
