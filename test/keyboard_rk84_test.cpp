#include <gtest/gtest.h>
#include "../include/main/keyboard.h"
#include <keyboards/rk84/rk84.h>

namespace KeyboardRK84
{
    // Test , returns the message_index 
    // to be 0 (the key is transmitted within the first message) and slot 9 of that message.
    TEST(KeyboardRK84, SetBytesInPacket_RK84WillStopAtNullKeyIds) {
        unsigned char onCode = 0x07;
        unsigned char offCode = 0x00;
        int offset = 5;

        unsigned char messages[3][65];
        char active_key_ids[] = { 0x01, 0x02, 0x00, 0x03 };

        RK84 rk_84;

        rk_84.SetBytesInPacket(*messages, kOn, active_key_ids, sizeof(active_key_ids));

        EXPECT_EQ(messages[0][offset + active_key_ids[0]], onCode);
        EXPECT_EQ(messages[0][offset + active_key_ids[1]], onCode);
        EXPECT_EQ(messages[0][offset], 0x01);  // This zero value of the offset should never be overwritten by SetBytesInPacket_RK84, it must be 0x01 per the packet definition
        EXPECT_EQ(messages[0][offset + active_key_ids[3]], offCode); // this won't be set to 0x03 because it's bad
    }
}

