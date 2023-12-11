#pragma once

#include "quantum_keycodes.h"
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

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[BASE] = LAYOUT_phoenix(
    KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_GRV,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_LCTL,
    KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_LSFT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,
                      KC_LSFT, KC_LCTL, KC_LALT,
                                                          KC_VOLD, KC_VOLU,
                                                 KC_SPC,  KC_LGUI, KC_LSFT,

             KC_EQL,  KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
             KC_EQL,  KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS,
             KC_MINS, KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
                      KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                               KC_SPC,  TG(1),   TG(2),
    KC_F11,  KC_F12,
    KC_ESC,  MO(1),   KC_ENT),
[ARROW] = LAYOUT_phoenix(
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TILD, KC_GRV,  KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                      KC_TRNS, KC_TRNS, KC_TRNS,
                                                          KC_TRNS, KC_TRNS,
                                                 KC_TRNS, KC_TRNS, KC_TRNS,

             KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_DEL,
             KC_TRNS, KC_TRNS, KC_PGDN, KC_PGUP, KC_LBRC, KC_RBRC, KC_TRNS,
             KC_TRNS, KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, KC_BSPC, KC_TRNS,
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS,  KC_TRNS,
    KC_TRNS,  KC_TRNS,   KC_TRNS),
[MOUSE] = LAYOUT_phoenix(
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                      KC_TRNS, KC_TRNS, KC_TRNS,
                                                          KC_TRNS, KC_TRNS,
                                                 KC_BTN1, KC_TRNS, KC_TRNS,

             KC_TRNS, KC_TRNS,       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
             KC_TRNS, KC_MS_WH_DOWN, KC_TRNS, KC_MS_U, KC_TRNS, KC_TRNS, KC_TRNS,
             KC_TRNS, KC_MS_WH_UP,   KC_MS_L, KC_MS_D, KC_MS_R, KC_TRNS, KC_TRNS,
                      KC_TRNS,       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                     KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS,  KC_TRNS,
    KC_TRNS,  KC_TRNS,   KC_TRNS),
};
