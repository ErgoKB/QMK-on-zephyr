#pragma once

#include "keycodes.h"
#include <stdint.h>

#define pgm_read_word(addr) (*(const unsigned short *)(addr))

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {[0] = {{KC_A}}};
