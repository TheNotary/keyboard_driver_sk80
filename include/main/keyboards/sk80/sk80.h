#pragma once
#include <array>

#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/sk80/constants_sk80.h"
#include "keyboards/sk80/messages_sk80.h"
#include "keyboards/sk80/key_mappings_sk80.h"


namespace sk80 {

class SK80 : public AbstractKeyboard {
public:
    SK80()
        : AbstractKeyboard(sk80::MESSAGE_LENGTH, sk80::BULK_LED_VALUE_MESSAGES_COUNT, sk80::target_device_path,
            DeviceInfo{ sk80::VID, sk80::PID }, sk80::keyname_keyid_mappings, sk80::max_key_id)
    {}

    void SetBytesInValuePackets(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys);


private:
    KeyValueBytesPair on_off_mappings = sk80::on_off_mappings;

};

}


namespace sk80::internal {

TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key);

}

