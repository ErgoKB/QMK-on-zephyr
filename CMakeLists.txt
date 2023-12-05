cmake_minimum_required(VERSION 3.20.0)

# Move to keyboard specific setting
add_compile_definitions(MATRIX_COLS=14)
add_compile_definitions(MATRIX_ROWS=5)
add_compile_definitions(MAX_LAYER=8)
add_compile_definitions(KEYBOARD_SHARED_EP)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(qmk_on_zephyr)

target_include_directories(
  app
  PRIVATE include/platforms/zephyr
          include/logging
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
          src/host.c
          src/bootloader.c
          src/keymap_introspection.c
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
          lib/qmk_firmware/quantum/keymap_common.c
          lib/qmk_firmware/quantum/keycode_config.c
          lib/qmk_firmware/quantum/eeconfig.c
          lib/qmk_firmware/quantum/bitwise.c
          lib/qmk_firmware/quantum/matrix_common.c
          lib/qmk_firmware/quantum/debounce/none.c
          lib/qmk_firmware/quantum/logging/print.c
          lib/qmk_firmware/quantum/logging/sendchar.c
          lib/qmk_firmware/quantum/logging/debug.c
          lib/qmk_firmware/quantum/bootmagic/magic.c)
