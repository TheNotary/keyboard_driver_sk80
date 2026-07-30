#pragma once
#include <unordered_map>
#include <string>
#include "misc.h"

namespace sk80 {

const short VID = 0x05ac;
const short PID = 0x024f;
const UINT8 max_key_id = 121;
const UINT8 MESSAGE_LENGTH = 65;
const UINT8 BULK_LED_VALUE_MESSAGES_COUNT = 9;
const UINT8 BULK_LED_HEADER_MESSAGES_COUNT = 2;
const UINT8 BULK_LED_FOOTER_MESSAGES_COUNT = 2;
const keylt::KeyValueBytesPair on_off_mappings = {
	{ keylt::kOn, 0xff },
	{ keylt::kOff, 0x00 }
};
// Both branches below must declare the same set of identifiers. A name that
// exists in only one branch compiles on one platform and fails on the other.
#ifdef _WIN32
const char target_device_path[] = "\\\\?\\hid#vid_05ac&pid_024f&mi_03#8&6cca243&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}";
// TODO: the Windows HID path for the LCD data interface has not been captured
// yet. The Windows LCD backend (SearchForLcdDataDevice / WriteDataToDevice /
// ReadFromDevice in usb_functions_win.cpp) is still a stub, so nothing reads
// this value at runtime; it exists so the shared code compiles.
const char lcd_data_device_path[] = "";
#else
// On Linux with hidapi-libusb, filter by interface number (matches mi_03 on Windows)
const char target_device_path[] = "3";
// LCD data interface — usage page 0xFF68 for raw write/read of image pages
const char lcd_data_device_path[] = "2";
#endif

// LCD display constants
const int LCD_W = 160;
const int LCD_H = 96;
const int LCD_PAGE_SIZE = 4096;
const int LCD_MAX_FRAMES = 141;

}
