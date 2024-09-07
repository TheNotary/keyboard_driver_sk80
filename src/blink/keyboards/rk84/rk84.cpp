#include "keyboards/rk84/rk84.h"
#include "keyboards/rk84/messages_rk84.h"
#include "misc.h"
#include <array>


TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId_RK84(UINT8 active_key) {
    if (active_key > 96) {
        throw("RK84 does not support keyIds greater than 96.");
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


//RK84::RK84()
//{
//}

void RK84::SetBytesInPacket(unsigned char* messages_ptr, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const
{
    // Cast the flat buffer to a 3x65 array
    unsigned char (*messages)[65] = reinterpret_cast<unsigned char (*)[65]>(messages_ptr);

    std::memcpy(messages, BULK_LED_VALUE_MESSAGES_RK84, BULK_LED_VALUE_MESSAGES_COUNT_RK84 * MESSAGE_LENGTH_RK84);

    char bytesForValue = on_off_mappings.at(key_value);

    for (int i = 0; i < n_active_keys; i++) {
        UINT8 active_key = active_key_ids[i];

        if (active_key == 0x00) // Stop writing when we reach a zero which terminates the buffer
            break;

        auto [message_index, keycode_offset] =
            GetMessageIndexAndKeycodeOffsetForKeyId_RK84(active_key);

        messages[message_index][keycode_offset] = bytesForValue;

        if (message_index == 1)  // the third page is always written to the same way the second page is written due to a bug I assume
            messages[2][keycode_offset] = bytesForValue;
    }
}

AbstractKeyboard::DeviceInfo RK84::GetDeviceInfo() const
{
    return this->device_info;
}
