#include <gtest/gtest.h>
#include "../include/main/keyboard.h"

namespace KeyboardRK84
{
    // void SetBytesInPacket_RK84(unsigned char messages[][65], KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) {
    // Test , returns the message_index 
    // to be 0 (the key is transmitted within the first message) and slot 9 of that message.
    TEST(KeyboardRK84, SetBytesInPacket_RK84WillStopAtNullKeyIds) {
        Keyboard keyboard(KeyboardModel::RK84);
        unsigned char onCode = 0x07;
        unsigned char offCode = 0x00;
        int offset = 5;

        unsigned char messages[3][65];
        char active_key_ids[] = { 0x01, 0x02, 0x00, 0x03 };

        SetBytesInPacket_RK84(messages, kOn, active_key_ids, sizeof(active_key_ids));

        EXPECT_EQ(messages[0][offset + active_key_ids[0]], onCode);
        EXPECT_EQ(messages[0][offset + active_key_ids[1]], onCode);
        EXPECT_EQ(messages[0][offset], 0x01);  // This zero value of the offset should never be overwritten by SetBytesInPacket_RK84, it must be 0x01 per the packet definition
        EXPECT_EQ(messages[0][offset + active_key_ids[3]], offCode); // this won't be set to 0x03 because it's bad
    }
}

