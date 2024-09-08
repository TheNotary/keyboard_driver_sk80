#pragma once
#include <array>
#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/constants_rk84.h"
#include "keyboards/rk84/messages_rk84.h"
#include "keyboards/rk84/key_mappings_rk84.h"


namespace rk84::internal {

    TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key);

}

namespace rk84 {

    class RK84 : public AbstractKeyboard {
    public:
        RK84()
            : AbstractKeyboard(rk84::MESSAGE_LENGTH, rk84::BULK_LED_VALUE_MESSAGES_COUNT, rk84::target_device_path, 
            DeviceInfo{ rk84::VID, rk84::PID }, rk84::keyname_keyid_mappings)
        {}

        void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys);
        DeviceInfo GetDeviceInfo() const;
        //DeviceInfo device_info;
        //std::unordered_map<std::string, char> keyname_keyid_mappings;
        // const char* target_device_path;


    private:
        // TODO: this isn't set the same way the other properties from const are set, update it if it makes sense for sk80 too
        KeyValueBytesPair on_off_mappings = rk84::on_off_mappings;

    };

}
