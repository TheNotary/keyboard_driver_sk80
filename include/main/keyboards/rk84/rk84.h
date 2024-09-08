#pragma once
#include <array>
#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/constants_rk84.h"

using namespace nRK84;


namespace nRK84 {
    TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId_RK84(UINT8 active_key);
}

class RK84 : public AbstractKeyboard {
public:
    RK84() 
        : AbstractKeyboard(MESSAGE_LENGTH, BULK_LED_VALUE_MESSAGES_COUNT),
        device_info({ VID, PID })
    {}
    void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys);
    DeviceInfo GetDeviceInfo() const;
    DeviceInfo device_info;

private:
    KeyValueBytesPair on_off_mappings = nRK84::on_off_mappings;

};
