#pragma once

#include <zephyr/usb/class/hid.h>
#include <zephyr/usb/usb_device.h>

#define RAW_EPSIZE 32

// clang-format off
static const uint8_t hid_kbd_report_desc[] = {
    // keyboard
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
    // mouse
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_REPORT_ID(2),
        HID_USAGE(HID_USAGE_GEN_DESKTOP_POINTER),
        HID_COLLECTION(HID_COLLECTION_PHYSICAL),
            // mouse buttons
            HID_USAGE_PAGE(HID_USAGE_GEN_BUTTON),
            HID_USAGE_MIN8(0x01),
            HID_USAGE_MAX8(0x08),
            HID_LOGICAL_MIN8(0),
            HID_LOGICAL_MAX8(1),
            HID_REPORT_COUNT(8),
            HID_REPORT_SIZE(1),
            HID_INPUT(0x02),
            // mouse position
            HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
            HID_USAGE(HID_USAGE_GEN_DESKTOP_X),
            HID_USAGE(HID_USAGE_GEN_DESKTOP_Y),
            HID_LOGICAL_MIN8(-127),
            HID_LOGICAL_MAX8(127),
            HID_REPORT_COUNT(2),
            HID_REPORT_SIZE(8),
            HID_INPUT(0x06),
            // wheel
            HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),
            HID_LOGICAL_MIN8(-127),
            HID_LOGICAL_MAX8(127),
            HID_REPORT_COUNT(1),
            HID_REPORT_SIZE(8),
            HID_INPUT(0x06),
        HID_END_COLLECTION,
    HID_END_COLLECTION,
    // system control
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(0x80),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_REPORT_ID(3),
        0x19, 0x01,       // Usage Minimum (Pointer)
        0x2A, 0xB7, 0x00, // Usage Maximum (Sys Display LCD Autoscale)
        0x15, 0x01,       // Logical Minimum (1)
        0x26, 0xB7, 0x00, // Logical Maximum (183)
        0x95, 0x01,       // Report Count (1)
        0x75, 0x10,       // Report Size (16)
        0x81, 0x00,       // Input (Data,Array,Abs,No Wrap,Linear,Preferred
                          // State,No Null Position)
    HID_END_COLLECTION,
    // consumer report
    HID_USAGE_PAGE(0x0C), // Usage Page (Consumer)
    0x09, 0x01,           // Usage (Consumer Control)
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_REPORT_ID(4),
        0x19, 0x01,       // Usage Minimum (Consumer Control)
        0x2A, 0xA0, 0x02, // Usage Maximum (0x02A0)
        0x15, 0x01,       // Logical Minimum (1)
        0x26, 0xA0, 0x02, // Logical Maximum (672)
        0x95, 0x01,       // Report Count (1)
        0x75, 0x10,       // Report Size (16)
        0x81, 0x00,       // Input (Data,Array,Abs,No Wrap,Linear,Preferred
                          // State,No Null Position)
    HID_END_COLLECTION,
};

static const uint8_t via_hid_report_desc[] = {
    0x06,
    0x60,
    0xFF, // vendor defined usage page
    HID_USAGE(0x61),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_USAGE(0x62),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX16(0xFF, 0x0),
        HID_REPORT_COUNT(32),
        HID_REPORT_SIZE(8),
        HID_INPUT(0x02),
        HID_USAGE(0x63),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX16(0xFF, 0x0),
        HID_REPORT_COUNT(32),
        HID_REPORT_SIZE(8),
        HID_OUTPUT(0x02),
    HID_END_COLLECTION,
};
// clang-format on
