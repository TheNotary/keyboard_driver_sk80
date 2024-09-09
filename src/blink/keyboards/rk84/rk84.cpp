#include "keyboards/rk84/rk84.h"

#include <array>

#include "keyboards/rk84/constants_rk84.h"
#include "misc.h"
#include "usb_functions.h"
#include "keyboard.h"

// This namespace makes TDD still possible, without muddying up the namespace disrupting drop-down menu style programming
namespace rk84::internal {

TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key) {
    if (active_key > ::rk84::max_key_id) {
        throw("RK84 does not support keyIds greater than " + ::rk84::max_key_id);
    }

    // TODO: Define this on the class just for fun, doesn't need to be on Abstract class too but maybe
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


namespace rk84 {

// TODO: this could become private, though I'd expect to define in in every keyboard anyway, so... idk
void RK84::SetBytesInValuePackets(unsigned char* messages_ptr, KeyValue key_value) {
    // Cast the flat buffer to a 3x65 array
    unsigned char (*messages)[65] = reinterpret_cast<unsigned char (*)[65]>(messages_ptr);
    
    std::memcpy(messages, rk84::BULK_LED_VALUE_MESSAGES, rk84::BULK_LED_VALUE_MESSAGES_COUNT * rk84::MESSAGE_LENGTH);

    char bytesForValue = on_off_mappings.at(key_value);

    for (int i = 0; i < this->keyboard_manager->n_active_keys; i++) {
        UINT8 active_key = this->keyboard_manager->GetActiveKeyId(i);

        if (active_key == 0x00) // Stop writing when we reach a zero which terminates the buffer
            break;

        auto [message_index, keycode_offset] =
            rk84::internal::GetMessageIndexAndKeycodeOffsetForKeyId(active_key);

        messages[message_index][keycode_offset] = bytesForValue;

        if (message_index == 1)  // the third page is always written to the same way the second page is written due to a bug I assume
            messages[2][keycode_offset] = bytesForValue;
    }
}

void RK84::SetKeysOnOff(KeyValue key_value, unsigned char* messages) {
    if (this->keyboard_manager->n_active_keys == 0) {
        printf("SetKeysOnOff was called with zero active keys... odd...skipping");
        return;
    }

    this->SetBytesInValuePackets(messages, key_value);

    if (settings::print_packets)
        PrintMessagesInBuffer(messages, this->BULK_LED_VALUE_MESSAGES_COUNT, this->MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, messages, this->BULK_LED_VALUE_MESSAGES_COUNT, this->MESSAGE_LENGTH);
}

void RK84::SetKeysRGB(unsigned char r, unsigned char g, unsigned char b)
{
    std::cerr << "Not supported" << std::endl;
    throw("Not supported");
}

}
