#include "keyboards/sk80/sk80.h"
#include "keyboards/sk80/constants_sk80.h"
#include "misc.h"
#include <array>

// This namespace makes TDD still possible, without muddying up the namespace disrupting drop-down menu style programming
namespace sk80::internal {

    TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key) {
        if (active_key > 96) {
            throw("SK80 does not support keyIds greater than n keys.");
        }

        UINT8 n_keys_in_first_packet = 59;
        UINT8 offset_to_message = active_key + 5;
        UINT8 message_index = 0;  // active_key = '.' aka
        if (active_key > n_keys_in_first_packet) {
            offset_to_message = active_key - n_keys_in_first_packet + 2;
            message_index = 1;
        }

        // This allows for C++17 destructuring via the auto keyword
        return { static_cast<UINT8>(message_index), static_cast<UINT8>(offset_to_message) };
    }

}


namespace sk80 {

    void SK80::SetBytesInPacket(unsigned char* messages_ptr, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys)
    {

        printf("You need to write this for the SK80!!!");
        throw("You need to write this for the SK80!!!");

        // Cast the flat buffer to a 3x65 array
        unsigned char (*messages)[65] = reinterpret_cast<unsigned char (*)[65]>(messages_ptr);

        std::memcpy(messages, sk80::BULK_LED_VALUE_MESSAGES, sk80::BULK_LED_VALUE_MESSAGES_COUNT * sk80::MESSAGE_LENGTH);

        char bytesForValue = on_off_mappings.at(key_value);

        for (int i = 0; i < n_active_keys; i++) {
            UINT8 active_key = active_key_ids[i];

            if (active_key == 0x00) // Stop writing when we reach a zero which terminates the buffer
                break;

            auto [message_index, keycode_offset] =
                sk80::internal::GetMessageIndexAndKeycodeOffsetForKeyId(active_key);

            messages[message_index][keycode_offset] = bytesForValue;

            if (message_index == 1)  // the third page is always written to the same way the second page is written due to a bug I assume
                messages[2][keycode_offset] = bytesForValue;
        }
    }

    AbstractKeyboard::DeviceInfo SK80::GetDeviceInfo() const
    {
        return this->device_info;
    }

}
