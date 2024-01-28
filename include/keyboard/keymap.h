#pragma once

#include "quantum_keycodes.h"
#include "keyboard.h"
#include <stdint.h>

#define pgm_read_word(addr) (*(const unsigned short *)(addr))

#define LAYOUT_phoenix(                    \
    k00,k01,k02,k03,k04,k05,k06,           \
    k10,k11,k12,k13,k14,k15,k16,           \
    k20,k21,k22,k23,k24,k25,k26,           \
    k30,k31,k32,k33,k34,k35,               \
            k42,k43,k44,                   \
                            k55,k56,       \
                        k53,k52,k51,       \
                                           \
        k07,k08,k09,k0A,k0B,k0C,k0D,       \
        k17,k18,k19,k1A,k1B,k1C,k1D,       \
        k27,k28,k29,k2A,k2B,k2C,k2D,       \
            k38,k39,k3A,k3B,k3C,k3D,       \
                k49,k4A,k4B,               \
    k57,k58,                               \
    k5C,k5B,k5A )                          \
                                           \
   /* matrix positions */                  \
   {                                       \
    { k00, k01, k02, k03, k04, k05, k06 }, \
    { k10, k11, k12, k13, k14, k15, k16 }, \
    { k20, k21, k22, k23, k24, k25, k26 }, \
    { k30, k31, k32, k33, k34, k35, k53 }, \
    { k52, k51, k42, k43, k44, k55, k56 }, \
                                           \
    { k0D, k0C, k0B, k0A, k09, k08, k07 }, \
    { k1D, k1C, k1B, k1A, k19, k18, k17 }, \
    { k2D, k2C, k2B, k2A, k29, k28, k27 }, \
    { k3D, k3C, k3B, k3A, k39, k38, k5A }, \
    { k5B, k5C, k4B, k4A, k49, k58, k57 }, \
   }

enum layers {
    BASE,  // default layer
    ARROW, // arrows
    MOUSE, // mouse keys
};

extern const uint16_t keymaps[MAX_LAYER][MATRIX_ROWS][MATRIX_COLS];
extern const keypos_t hand_swap_config[MATRIX_ROWS][MATRIX_COLS];
