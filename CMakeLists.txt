cmake_minimum_required(VERSION 3.20.0)

add_compile_definitions(MATRIX_COLS=CONFIG_MATRIX_COLS)
add_compile_definitions(MATRIX_ROWS=CONFIG_MATRIX_ROWS)
add_compile_definitions(MAX_LAYER=CONFIG_MAX_LAYER)
add_compile_definitions(RAW_ENABLE=CONFIG_RAW_ENABLE)
add_compile_definitions(KEYBOARD_SHARED_EP)
add_compile_definitions(MOUSE_ENABLE=CONFIG_MOUSE_ENABLE)
add_compile_definitions(MOUSE_SHARED_EP)
add_compile_definitions(MOUSEKEY_ENABLE=CONFIG_MOUSE_ENABLE)
add_compile_definitions(EXTRAKEY_ENABLE=CONFIG_EXTRAKEY_ENABLE)
add_compile_definitions(KEYMAP_C="keymap.h")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(qmk_on_zephyr)

target_include_directories(
  app
  PRIVATE include
          include/platforms/zephyr
          include/logging
          include/keyboard
          lib/qmk_firmware/tmk_core/protocol
          lib/qmk_firmware/quantum
          lib/qmk_firmware/quantum/bootmagic
          lib/qmk_firmware/quantum/logging
          lib/qmk_firmware/quantum/keymap_extras
          lib/qmk_firmware/quantum/sequencer
          lib/qmk_firmware/platforms)

target_sources(
  app
  PRIVATE src/main.c
          src/protocol.c
          src/timer.c
          src/eeprom.c
          src/matrix.c
          src/host.c
          src/bootloader.c
          src/usb_main.c
          lib/qmk_firmware/tmk_core/protocol/report.c
          lib/qmk_firmware/tmk_core/protocol/usb_device_state.c
          lib/qmk_firmware/quantum/quantum.c
          lib/qmk_firmware/quantum/keyboard.c
          lib/qmk_firmware/quantum/led.c
          lib/qmk_firmware/quantum/action.c
          lib/qmk_firmware/quantum/action_util.c
          lib/qmk_firmware/quantum/action_layer.c
          lib/qmk_firmware/quantum/action_tapping.c
          lib/qmk_firmware/quantum/action_tapping.c
          lib/qmk_firmware/quantum/keymap_common.c
          lib/qmk_firmware/quantum/keymap_introspection.c
          lib/qmk_firmware/quantum/keycode_config.c
          lib/qmk_firmware/quantum/eeconfig.c
          lib/qmk_firmware/quantum/bitwise.c
          lib/qmk_firmware/quantum/matrix_common.c
          lib/qmk_firmware/quantum/debounce/none.c
          lib/qmk_firmware/quantum/logging/print.c
          lib/qmk_firmware/quantum/logging/sendchar.c
          lib/qmk_firmware/quantum/logging/debug.c
          lib/qmk_firmware/quantum/mousekey.c
          lib/qmk_firmware/quantum/bootmagic/magic.c)

add_subdirectory(src/split)
