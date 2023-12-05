#pragma once

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/hid.h>

// clang-format off
static const uint8_t hid_kbd_report_desc[] = {
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(HID_USAGE_GEN_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_REPORT_ID(1),
        HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP_KEYPAD),
        HID_USAGE_MIN8(0xE0),
        HID_USAGE_MAX8(0xE7),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX8(1),
        // First byte: modifier
        HID_REPORT_SIZE(1),
        HID_REPORT_COUNT(8),
        HID_INPUT(0x02),
        HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP_KEYPAD),
        // Second byte: reserved 
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(1),
        HID_INPUT(0x03),
        HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP_KEYPAD),
        // Rest bytes
        HID_USAGE_MIN8(0x00),
        HID_USAGE_MAX8(0xFF),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX8(0xFF),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(6),
        HID_INPUT(0x00),
    HID_END_COLLECTION,
};
// clang-format on
