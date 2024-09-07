#pragma once
#include <array>
#include "misc.h"
#include "keyboards/abstract_keyboard.h"


TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId_RK84(UINT8 active_key);

// TODO: 
//   - Create a base class and inherit from that
//   - Inject this into the main Keyboard class during the initializer.

class RK84 : public AbstractKeyboard {
public:
	 void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const;

private:
    KeyValueBytesPair on_off_mappings = {
        { kOn, 0x07 },
        { kOff, 0x00 }
    };

    DeviceInfo device_mappings = { 0x258a, 0x00c0 };

};
