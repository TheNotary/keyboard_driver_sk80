#pragma once
#include <array>

#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/constants_rk84.h"
#include "keyboards/rk84/messages_rk84.h"
#include "keyboards/rk84/key_mappings_rk84.h"


namespace rk84 {


class RK84 : public keylt::AbstractKeyboard {
public:
    RK84()
        : keylt::AbstractKeyboard(rk84::MESSAGE_LENGTH, rk84::BULK_LED_VALUE_MESSAGES_COUNT, rk84::target_device_path,
            DeviceInfo{ rk84::VID, rk84::PID }, rk84::keyname_keyid_mappings, rk84::max_key_id, nullptr)
    {}

    RK84(keylt::Keyboard* keyboard_manager)
        : keylt::AbstractKeyboard(rk84::MESSAGE_LENGTH, rk84::BULK_LED_VALUE_MESSAGES_COUNT, rk84::target_device_path,
        DeviceInfo{ rk84::VID, rk84::PID }, rk84::keyname_keyid_mappings, rk84::max_key_id, keyboard_manager)
    {}

    void SetBytesInValuePackets(unsigned char* messages, keylt::KeyValue key_value);
    void SetKeysOnOff(keylt::KeyValue key_value, unsigned char* messages);

    void SetKeysRGB(unsigned char r, unsigned char g, unsigned char b);


private:
    // TODO: this isn't set the same way the other properties from const are set, update it if it makes sense for sk80 too
    keylt::KeyValueBytesPair on_off_mappings = rk84::on_off_mappings;

};


}


namespace rk84::internal {

keylt::TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key);

}