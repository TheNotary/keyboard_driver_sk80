#include "keyboards/sk80/sk80.h"
#include "keyboards/sk80/constants_sk80.h"
#include "misc.h"
#include <array>

//{ // 1 - Top Row, esc - f12
//    0x00,
//        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // KeyId 0x00 - 0x0f
//        0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
//        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
//        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
//        0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
//        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
//        0x0c, 0x00, 0x00, 0x00, 0x0d, 0xff, 0x00, 0x00,
//        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // technically 16 keys per message, though key 0x00 is not bound
//    },


// This namespace makes TDD still possible, without muddying up the namespace disrupting drop-down menu style programming
namespace sk80::internal {

    TwoUINT8s GetMessageIndexAndKeycodeOffsetForKeyId(UINT8 active_key) {
        if (active_key > sk80::max_key_id) {
            throw("SK80 does not support keyIds greater than " + sk80::max_key_id);
        }

        // assume 0x0f, 15, which is   4 * 15 + 1 = index:61
        // Also consider 0x10, 16, which is message_index:2

        UINT8 n_keys_in_first_packet = 15;
        UINT8 width_of_key = 4;
        UINT8 message_index = (active_key - 1) / n_keys_in_first_packet;        // when active_key = 1, 0 % 15 is zero, so 0 * 4
        UINT8 offset_to_message = ((active_key - 1) % 15) * width_of_key + 1;   // when active_key = 16,  15 % 15 is zero, so 0 * width_of_key = 0
        if (message_index == 0)
            offset_to_message += 4;  // we'll count the first packet as zero indexed and everything will work out, but this register massaging is because they did their math in creative ways

        // This allows for C++17 destructuring via the auto keyword
        return { static_cast<UINT8>(message_index), static_cast<UINT8>(offset_to_message) };
    }

    static bool is_template_message_missing_requested_key(unsigned char key_id_in_messages_slot, UINT8 active_key_id) {
        return (key_id_in_messages_slot != active_key_id);
    }

}


namespace sk80 {

    void SK80::SetBytesInValuePackets(unsigned char* messages_ptr, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys)
    {
        // Cast the flat buffer to a 3x65 array
        unsigned char (*messages)[sk80::MESSAGE_LENGTH] = reinterpret_cast<unsigned char (*)[sk80::MESSAGE_LENGTH]>(messages_ptr);

        std::memcpy(messages, sk80::BULK_LED_VALUE_MESSAGES, sk80::BULK_LED_VALUE_MESSAGES_COUNT * sk80::MESSAGE_LENGTH);
            
        char bytesForValue = on_off_mappings.at(key_value); 

        for (int i = 0; i < n_active_keys; i++) {
            UINT8 active_key_id = active_key_ids[i];

            if (active_key_id == 0x00) // Stop writing when we reach a zero which terminates the buffer
                break;

            auto [message_index, keycode_offset] =
                internal::GetMessageIndexAndKeycodeOffsetForKeyId(active_key_id);

            if (internal::is_template_message_missing_requested_key(messages[message_index][keycode_offset], active_key_id)) {
                std::cerr << "KeyID not registered with this keyboard, skipping " << active_key_id << ". " << std::endl;
                continue;
            }

            // Set key id... though technically there's no need to do this since the key_ids are hardcoded in the template messages
            // messages[message_index][keycode_offset] = active_key_id;

            // Set the color values
            messages[message_index][keycode_offset + 1] = bytesForValue;
            messages[message_index][keycode_offset + 2] = bytesForValue;
            messages[message_index][keycode_offset + 3] = bytesForValue;
        }
    }

}
