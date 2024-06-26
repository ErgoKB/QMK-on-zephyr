#include "quantum_keycodes.h"
#include "keycode.h"
#include "keyboard.h"
#include "keymap.h"

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
                               KC_SPC,  (QK_TOGGLE_LAYER | ((1)&0xFF)),   QK_TOGGLE_LAYER | ((2)&0xFF),
    KC_F11,  KC_F12,
    KC_ESC,  (QK_MOMENTARY | ((1)&0xFF)),   KC_ENT),
[ARROW] = LAYOUT_phoenix(
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_GRV, KC_GRV,  KC_TRNS,
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

const keypos_t hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
    { { 0, 5 }, { 1, 5 }, { 2, 5 }, { 3, 5 }, { 4, 5 }, { 5, 5 }, { 6, 5 } },
    { { 0, 6 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 6 }, { 6, 6 } },
    { { 0, 7 }, { 1, 7 }, { 2, 7 }, { 3, 7 }, { 4, 7 }, { 5, 7 }, { 6, 7 } },
    { { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 6, 8 } },
    { { 0, 9 }, { 1, 9 }, { 2, 9 }, { 3, 9 }, { 4, 9 }, { 5, 9 }, { 6, 9 } },
    { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 0 } },
    { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 1 }, { 6, 1 } },
    { { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 2 }, { 5, 2 }, { 6, 2 } },
    { { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 }, { 5, 3 }, { 6, 3 } },
    { { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 } },
};
