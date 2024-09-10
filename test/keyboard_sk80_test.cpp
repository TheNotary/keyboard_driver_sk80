#include <gtest/gtest.h>
#include "../include/main/keyboard.h"
#include <keyboards/sk80/sk80.h>

using namespace sk80;
using namespace blink;

namespace KeyboardSK80
{
    class KeyboardSK80Test : public ::testing::Test {
    protected:
        unsigned char onCode = 0xff;
        unsigned char offCode = 0x00;
        unsigned char messages[sk80::BULK_LED_VALUE_MESSAGES_COUNT][sk80::MESSAGE_LENGTH] = { 0 };
        Keyboard keyboard_manager;
        SK80* sk_80;

        KeyboardSK80Test() : keyboard_manager(kSK80) {}

        void SetUp() override {
            sk_80 = dynamic_cast<SK80*>(keyboard_manager.keyboard_spec);
        }

        void SetUpWithKeyIds(const char* active_key_ids, size_t key_ids_count) {
            keyboard_manager.SetActiveKeyIds(active_key_ids, key_ids_count);
        }
    };

     TEST_F(KeyboardSK80Test, SetBytesInPacket_SK80WillStopAtNullKeyIds) {
         char active_key_ids[] = { 0x01, 0x02, 0x00, 0x03 };
         SetUpWithKeyIds(active_key_ids, std::size(active_key_ids));

         sk_80->SetBytesInValuePackets(*messages, kOn);

         int header_offset = 1;
         int i, key_offset;

         // escape key should be on
         i = 0;
         key_offset = 4;  // key_id * 4 for the first page...
         EXPECT_EQ(messages[0][header_offset + key_offset + 0], active_key_ids[i]);
         EXPECT_EQ(messages[0][header_offset + key_offset + 1], onCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 2], onCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 3], onCode);

         // f1 key should be on
         i = 1;
         key_offset = 8; 
         EXPECT_EQ(messages[0][header_offset + key_offset + 0], active_key_ids[i]);
         EXPECT_EQ(messages[0][header_offset + key_offset + 1], onCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 2], onCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 3], onCode);

         // The null key should always be off
         i = 2;
         key_offset = 0;
         EXPECT_EQ(messages[0][header_offset + key_offset + 0], active_key_ids[i]);
         EXPECT_EQ(messages[0][header_offset + key_offset + 1], offCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 2], offCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 3], offCode);

         // f2 key should be off.  Even though f2, 0x03 is set in active_key_ids, active_key_ids was null terminated at index 2
         i = 3;
         key_offset = 12;
         EXPECT_EQ(messages[0][header_offset + key_offset + 0], active_key_ids[i]);
         EXPECT_EQ(messages[0][header_offset + key_offset + 1], offCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 2], offCode);
         EXPECT_EQ(messages[0][header_offset + key_offset + 3], offCode);
     }

    TEST_F(KeyboardSK80Test, SetBytesInPacket_PutsVariousKeysInExpectedSpots) {
        char active_key_ids[] = { 1, 13, 19, 20, 21, 37 };
        SetUpWithKeyIds(active_key_ids, std::size(active_key_ids));

        sk_80->SetBytesInValuePackets(*messages, kOn);

        int offset = 1;
        int i;

        // Escape key
        i = 0;
        EXPECT_EQ(messages[0][offset + 0 + active_key_ids[i] * 4], active_key_ids[i]);
        EXPECT_EQ(messages[0][offset + 1 + active_key_ids[i] * 4], onCode);
        EXPECT_EQ(messages[0][offset + 2 + active_key_ids[i] * 4], onCode);
        EXPECT_EQ(messages[0][offset + 3 + active_key_ids[i] * 4], onCode);

        // 1 key
        i = 1;
        EXPECT_EQ(messages[0][offset + 0 + active_key_ids[i] * 4], active_key_ids[i]); //
        EXPECT_EQ(messages[0][offset + 1 + active_key_ids[i] * 4], onCode);
        EXPECT_EQ(messages[0][offset + 2 + active_key_ids[i] * 4], onCode);
        EXPECT_EQ(messages[0][offset + 3 + active_key_ids[i] * 4], onCode);

        // idk key, second page
        i = 2;
        EXPECT_EQ(messages[1][offset + 0 + 3 * 4], active_key_ids[i]);
        EXPECT_EQ(messages[1][offset + 1 + 3 * 4], onCode);
        EXPECT_EQ(messages[1][offset + 2 + 3 * 4], onCode);
        EXPECT_EQ(messages[1][offset + 3 + 3 * 4], onCode);

        // idk key, second page
        i = 3;
        EXPECT_EQ(messages[1][offset + 0 + 4 * 4], active_key_ids[i]);
        EXPECT_EQ(messages[1][offset + 1 + 4 * 4], onCode);
        EXPECT_EQ(messages[1][offset + 2 + 4 * 4], onCode);
        EXPECT_EQ(messages[1][offset + 3 + 4 * 4], onCode);

        // 4 key, this should be off since it's not specified
        i = 4;
        EXPECT_EQ(messages[1][offset + 0 + 5 * 4], active_key_ids[i]);
        EXPECT_EQ(messages[1][offset + 1 + 5 * 4], onCode);
        EXPECT_EQ(messages[1][offset + 2 + 5 * 4], onCode);
        EXPECT_EQ(messages[1][offset + 3 + 5 * 4], onCode);

        // `=` key, this should be on page 3
        i = 5;
        EXPECT_EQ(messages[2][offset + 0 + 5 * 4], active_key_ids[i]);
        EXPECT_EQ(messages[2][offset + 1 + 5 * 4], onCode);
        EXPECT_EQ(messages[2][offset + 2 + 5 * 4], onCode);
        EXPECT_EQ(messages[2][offset + 3 + 5 * 4], onCode);
    }

    TEST_F(KeyboardSK80Test, SetBytesInPacket_KeyId121Works) {
        char active_key_ids[] = { 121 };
        SetUpWithKeyIds(active_key_ids, std::size(active_key_ids));

        sk_80->SetBytesInValuePackets(*messages, kOn);

        // assert escape key is written to packet
        int offset = 1 + 9 * 4;
        EXPECT_EQ(messages[7][offset + 0], active_key_ids[0]);
        EXPECT_EQ(messages[7][offset + 1], onCode);
        EXPECT_EQ(messages[7][offset + 2], onCode);
        EXPECT_EQ(messages[7][offset + 3], onCode);
    }

    TEST(KeyboardSK80Attributes, ItHasTheCorrectConstsAvailable) {
        SK80 sk_80;
        AbstractKeyboard* abstr = &sk_80;

        EXPECT_EQ(sk_80.BULK_LED_VALUE_MESSAGES_COUNT, 9);
        EXPECT_EQ(abstr->BULK_LED_VALUE_MESSAGES_COUNT, 9);
        EXPECT_EQ(sk_80.MESSAGE_LENGTH, 65);
        EXPECT_EQ(abstr->MESSAGE_LENGTH, 65);
        EXPECT_EQ(sk_80.keyname_keyid_mappings["tab"], 37);
        EXPECT_EQ(abstr->keyname_keyid_mappings["tab"], 37);
        EXPECT_EQ(sk_80.target_device_path[0], '\\');
        EXPECT_EQ(abstr->target_device_path[0], '\\');
        EXPECT_EQ(sk_80.device_info.vid, 0x05ac);
        EXPECT_EQ(abstr->device_info.vid, 0x05ac);
        EXPECT_EQ(sk_80.device_info.pid, 0x024f);
        EXPECT_EQ(abstr->device_info.pid, 0x024f);
        EXPECT_EQ(sk_80.max_key_id, 121);
        EXPECT_EQ(abstr->max_key_id, 121);
    }

}
