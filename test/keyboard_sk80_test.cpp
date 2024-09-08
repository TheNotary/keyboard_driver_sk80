#include <gtest/gtest.h>
#include "../include/main/keyboard.h"
#include <keyboards/sk80/sk80.h>

using namespace sk80;

namespace KeyboardSK80
{
    // Test , returns the message_index
    // to be 0 (the key is transmitted within the first message) and slot 9 of that message.
    // TEST(KeyboardSK80, SetBytesInPacket_SK80WillStopAtNullKeyIds) {
    //     unsigned char onCode = 0x07;
    //     unsigned char offCode = 0x00;
    //     int offset = 5;

    //     unsigned char messages[3][65];
    //     char active_key_ids[] = { 0x01, 0x02, 0x00, 0x03 };

    //     SK80 sk_80;

    //     sk_80.SetBytesInPacket(*messages, kOn, active_key_ids, sizeof(active_key_ids));

    //     EXPECT_EQ(messages[0][offset + active_key_ids[0]], onCode);
    //     EXPECT_EQ(messages[0][offset + active_key_ids[1]], onCode);
    //     EXPECT_EQ(messages[0][offset], 0x01);  // This zero value of the offset should never be overwritten by SetBytesInPacket_SK80, it must be 0x01 per the packet definition
    //     EXPECT_EQ(messages[0][offset + active_key_ids[3]], offCode); // this won't be set to 0x03 because it's bad
    // }

    TEST(KeyboardSK80, ItHasTheCorrectConstsAvailable) {
        SK80 sk_80;

        AbstractKeyboard* abstr = &sk_80;

        EXPECT_EQ(sk_80.BULK_LED_VALUE_MESSAGES_COUNT, 9);
        EXPECT_EQ(abstr->BULK_LED_VALUE_MESSAGES_COUNT, 9);
        EXPECT_EQ(sk_80.MESSAGE_LENGTH, 65);
        EXPECT_EQ(abstr->MESSAGE_LENGTH, 65);
        EXPECT_EQ(sk_80.keyname_keyid_mappings["tab"], 3);
        EXPECT_EQ(abstr->keyname_keyid_mappings["tab"], 3);
        EXPECT_EQ(sk_80.target_device_path[0], '\\');
        EXPECT_EQ(abstr->target_device_path[0], '\\');
        EXPECT_EQ(sk_80.device_info.vid, 0x05ac);
        EXPECT_EQ(abstr->device_info.vid, 0x05ac);
        EXPECT_EQ(sk_80.device_info.pid, 0x024f);
        EXPECT_EQ(abstr->device_info.pid, 0x024f);
    }

}

