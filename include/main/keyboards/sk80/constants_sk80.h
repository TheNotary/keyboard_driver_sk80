#pragma once
#include <unordered_map>
#include <string>
#include "misc.h"

namespace sk80 {

const short VID = 0x05ac;
const short PID = 0x024f;
const UINT8 max_key_id = 79;
const UINT8 MESSAGE_LENGTH = 65;
const UINT8 BULK_LED_HEADER_MESSAGES_COUNT = 2;
const UINT8 BULK_LED_VALUE_MESSAGES_COUNT = 9;
const UINT8 BULK_LED_FOOTER_MESSAGES_COUNT = 2;
const KeyValueBytesPair on_off_mappings = {
	{ kOn, 0xff },
	{ kOff, 0x00 }
};
const char target_device_path[] = "\\\\?\\hid#vid_05ac&pid_024f&mi_03#8&6cca243&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}";

}
