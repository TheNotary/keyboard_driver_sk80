#pragma once
#include <array>
#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/constants_rk84.h"
#include "keyboards/rk84/messages_rk84.h"

namespace rk84::internal {

    TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key);

}

namespace rk84 {

    class RK84 : public AbstractKeyboard {
    public:
        RK84() 
            : AbstractKeyboard(rk84::MESSAGE_LENGTH, rk84::BULK_LED_VALUE_MESSAGES_COUNT),
            device_info({ VID, PID })
        {}
        void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys);
        DeviceInfo GetDeviceInfo() const;
        DeviceInfo device_info;

    private:
        KeyValueBytesPair on_off_mappings = rk84::on_off_mappings;

    };

}
