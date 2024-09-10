#include <array>

#include "misc.h"
#include "keyboard.h"
#include "usb_functions.h"
#include "keyboards/sk80/sk80.h"
#include "keyboards/sk80/constants_sk80.h"
#include "messages.h"

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

        UINT8 n_keys_in_first_packet = 16;
        UINT8 width_of_key = 4;
        UINT8 message_index = active_key / n_keys_in_first_packet;
        UINT8 offset_to_message = (active_key % 16) * width_of_key + 1;

        // This allows for C++17 destructuring via the auto keyword
        return { static_cast<UINT8>(message_index), static_cast<UINT8>(offset_to_message) };
    }

    static bool is_template_message_missing_requested_key(unsigned char key_id_in_messages_slot, UINT8 active_key_id) {
        return (key_id_in_messages_slot != active_key_id);
    }

}


namespace sk80 {

    void SK80::SetBytesInValuePackets(unsigned char* messages_ptr, KeyValue key_value)
    {
        // A 9x65 array cast from the flat buffer
        unsigned char (&messages)[sk80::BULK_LED_VALUE_MESSAGES_COUNT][sk80::MESSAGE_LENGTH] = *reinterpret_cast<unsigned char (*)[sk80::BULK_LED_VALUE_MESSAGES_COUNT][sk80::MESSAGE_LENGTH]>(messages_ptr);

        std::memcpy(messages, sk80::BULK_LED_VALUE_MESSAGES, sk80::BULK_LED_VALUE_MESSAGES_COUNT * sk80::MESSAGE_LENGTH);
        
        char bytesForValue = on_off_mappings.at(key_value);

        for (int i = 0; i < this->keyboard_manager->n_active_keys; i++) {
            UINT8 active_key_id = this->keyboard_manager->GetActiveKeyId(i);

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

    void SK80::SetKeysOnOff(KeyValue key_value, unsigned char* messages) {
        if (this->keyboard_manager->n_active_keys == 0) {
            printf("SetKeysOnOff was called with zero active keys... odd...skipping");
            return;
        }

        //PrintMessagesInBuffer(*sk80::BULK_LED_HEADER_MESSAGES, sk80::BULK_LED_HEADER_MESSAGES_COUNT, sk80::MESSAGE_LENGTH);
        SendBufferToDeviceAndGetResp(this->device_handle, *sk80::BULK_LED_HEADER_MESSAGES, sk80::BULK_LED_HEADER_MESSAGES_COUNT, sk80::MESSAGE_LENGTH);

        this->SetBytesInValuePackets(messages, key_value);
        //PrintMessagesInBuffer(messages, sk80::BULK_LED_VALUE_MESSAGES_COUNT, sk80::MESSAGE_LENGTH);
        SendBufferToDevice(this->device_handle, messages, sk80::BULK_LED_VALUE_MESSAGES_COUNT, sk80::MESSAGE_LENGTH);
        
        //PrintMessagesInBuffer(*sk80::BULK_LED_FOOTER_MESSAGES, sk80::BULK_LED_FOOTER_MESSAGES_COUNT, sk80::MESSAGE_LENGTH);
        SendBufferToDeviceAndGetResp(this->device_handle, *sk80::BULK_LED_FOOTER_MESSAGES, sk80::BULK_LED_FOOTER_MESSAGES_COUNT, sk80::MESSAGE_LENGTH);
    }

    void SK80::SetKeyRGB(char key_id, unsigned char r, unsigned char g, unsigned char b) {
        std::cout << "Setting LED" << std::endl;

        SendBufferToDeviceAndGetResp(this->device_handle, *TEST_SLIM_HEADER_MESSAGES, 2, MESSAGE_LENGTH);

        TEST_SLIM_MESSAGES[0][54] = r;
        TEST_SLIM_MESSAGES[0][55] = g;
        TEST_SLIM_MESSAGES[0][56] = b;

        SendBufferToDevice(this->device_handle, *TEST_SLIM_MESSAGES, 1, MESSAGE_LENGTH);

        SendBufferToDevice(this->device_handle, *END_BULK_UPDATE_MESSAGES, END_BULK_UPDATE_MESSAGE_COUNT, MESSAGE_LENGTH);
    }

    void SK80::SetKeysRGB(unsigned char r, unsigned char g, unsigned char b) {
        if (keyboard_manager->n_active_keys == 0) {
            printf("SetKeysRGB was called with zero active keys... odd...");
            return;
        }

        //char key_message[65] = {
        //    0x00,
        //    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        //    0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        //    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        //    0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        //    0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        //    0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        //    0x0c, 0x00, 0x00, 0x00, 0x0d, 0xff, 0x00, 0x00,
        //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        //};

        unsigned char key_message[1][65] = { 0x00 };

        UINT8 n_packets = ((keyboard_manager->n_active_keys - 1u) / 15u) + 1u;

        // we can only have 15 colors per page
        // What about this, for every active key, let's pull up the index it should be in the first packet
        // and send it...
        for (int i = 0; i < keyboard_manager->n_active_keys; i++) {
            UINT8 active_key = keyboard_manager->GetActiveKeyId(i);
            // for 0x01, we need to insert into i = 0; offset = 

            UINT8 offset = 5 + ((active_key - 1) * 4);

            key_message[0][offset + 0] = keyboard_manager->GetActiveKeyId(i);
            key_message[0][offset + 1] = r;
            key_message[0][offset + 2] = g;
            key_message[0][offset + 3] = b;
        }

        printf("Sending RGB message: \n");
        printf("0x00");
        for (int i = 1; i < this->MESSAGE_LENGTH; i++) {
            if ((i - 1) % 8 == 0)
                printf("\n");
            printf("0x%02x ", key_message[0][i]);
        }
        printf("\n");

        SendBufferToDeviceAndGetResp(this->device_handle, *TEST_SLIM_HEADER_MESSAGES, 2, this->MESSAGE_LENGTH);

        SendBufferToDevice(this->device_handle, *key_message, 1, this->MESSAGE_LENGTH);

        unsigned char msg2[1][65] = { // 2
            0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x13, 0xff, 0x00, 0x00,
            0x14, 0xff, 0x00, 0x00, 0x15, 0xff, 0x00, 0x00,
            0x16, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
            0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
            0x1a, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00,
            0x1c, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
            0x1e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00
        };

        SendBufferToDevice(this->device_handle, *msg2, 1, this->MESSAGE_LENGTH);

        //SendBufferToDevice(this->device_handle, TEST_SLIM_MESSAGES, 1, MESSAGE_LENGTH);

        SendBufferToDevice(this->device_handle, *END_BULK_UPDATE_MESSAGES, END_BULK_UPDATE_MESSAGE_COUNT, this->MESSAGE_LENGTH);
    }

}
