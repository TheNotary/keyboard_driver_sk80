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
}


// FIXME: I currently have a circular dependency so I can't define anything other than a const in this file or I will get errors that
// I'm redefining attributes
//
// To solve this, it's clever to instead focus on removing the need to include 
// #include "keyboards/rk84/messages_rk84.h"
// from within keyboard.cpp
// That means I need to define the data on the RK84 class and access the data through that class
//
// But for that to work out, I need to move all the function calls that turn lights on/ off which use those message buffers, 
// so that the KB84 class never needs to expose those members which would really mess up the abstract_keyboard class declaration

