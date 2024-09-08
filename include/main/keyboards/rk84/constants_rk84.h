#pragma once
#include <unordered_map>
#include <string>
#include "misc.h"


namespace rk84 {
	const short VID = 0x258a;
	const short PID = 0x00c0;
	const UINT8 MESSAGE_LENGTH = 65;
	const UINT8 BULK_LED_VALUE_MESSAGES_COUNT = 3;
	const KeyValueBytesPair on_off_mappings = {
		{ kOn, 0x07 },
		{ kOff, 0x00 }
	};
	const char target_device_path[] = "\\\\?\\hid#vid_258a&pid_00c0&mi_01&col05#9&3b698677&0&0004#{4d1e55b2-f16f-11cf-88cb-001111000030}";
}
