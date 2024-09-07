#pragma once
#include <array>
#include "misc.h"
#include "keyboards/abstract_keyboard.h"


TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId_RK84(UINT8 active_key);


namespace nRK84 {
    const short VID = 0x258a;
    const short PID = 0x00c0;
    const UINT8 MESSAGE_LENGTH = 65;
    const UINT8 BULK_LED_VALUE_MESSAGES_COUNT = 3;
}

// TODO: 
//   - Create a base class and inherit from that
//   - Inject this into the main Keyboard class during the initializer.

class RK84 : public AbstractKeyboard {
public:
    RK84() 
        : AbstractKeyboard(nRK84::MESSAGE_LENGTH, nRK84::BULK_LED_VALUE_MESSAGES_COUNT),
        device_info({ nRK84::VID, nRK84::PID })
    {}
    void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const;
    DeviceInfo GetDeviceInfo() const;
    DeviceInfo device_info = { 0x258a, 0x00c0 };

private:
    KeyValueBytesPair on_off_mappings = {
        { kOn, 0x07 },
        { kOff, 0x00 }
    };

};

