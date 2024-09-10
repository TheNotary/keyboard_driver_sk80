#pragma once
#include <array>

#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/sk80/constants_sk80.h"
#include "keyboards/sk80/messages_sk80.h"
#include "keyboards/sk80/key_mappings_sk80.h"


namespace sk80 {


class SK80 : public blink::AbstractKeyboard {
public:
    SK80()
        : blink::AbstractKeyboard(sk80::MESSAGE_LENGTH, sk80::BULK_LED_VALUE_MESSAGES_COUNT, sk80::target_device_path,
            DeviceInfo{ sk80::VID, sk80::PID }, sk80::keyname_keyid_mappings, sk80::max_key_id, nullptr)
    {}

    SK80(blink::Keyboard* keyboard_manager)
        : AbstractKeyboard(sk80::MESSAGE_LENGTH, sk80::BULK_LED_VALUE_MESSAGES_COUNT, sk80::target_device_path,
        DeviceInfo{ sk80::VID, sk80::PID }, sk80::keyname_keyid_mappings, sk80::max_key_id, keyboard_manager)
    {}
    
    void SetBytesInValuePackets(unsigned char* messages, blink::KeyValue key_value);
    void SetKeysOnOff(blink::KeyValue key_value, unsigned char* messages);

    void SetKeyRGB(char key_id, unsigned char r, unsigned char g, unsigned char b);
    void SetKeysRGB(unsigned char r, unsigned char g, unsigned char b);

private:
    blink::KeyValueBytesPair on_off_mappings = sk80::on_off_mappings;

};

}


namespace sk80::internal {

blink::TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key);

}
