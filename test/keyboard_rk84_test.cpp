#include <gtest/gtest.h>
#include "../include/main/keyboard.h"
#include <keyboards/rk84/rk84.h>

using namespace rk84;
using namespace blink;

namespace KeyboardRK84
{
    class KeyboardRK84Test : public ::testing::Test {
    protected:
        unsigned char onCode = 0x07;
        unsigned char offCode = 0x00;
        unsigned char messages[rk84::BULK_LED_VALUE_MESSAGES_COUNT][rk84::MESSAGE_LENGTH] = { 0 };
        Keyboard keyboard_manager;
        RK84* rk_84;

        KeyboardRK84Test() : keyboard_manager(kRK84) {}

        void SetUp() override {
            rk_84 = dynamic_cast<RK84*>(keyboard_manager.keyboard_spec);
        }

        void SetUpWithKeyIds(const char* active_key_ids, size_t key_ids_count) {
            keyboard_manager.SetActiveKeyIds(active_key_ids, key_ids_count);
        }
    };

    TEST_F(KeyboardRK84Test, SetBytesInPacket_RK84WillStopAtNullKeyIds) {
        char active_key_ids[] = { 0x01, 0x02, 0x00, 0x03 };
        SetUpWithKeyIds(active_key_ids, std::size(active_key_ids));

        rk_84->SetBytesInValuePackets(*messages, kOn);

        // test that only two keys in the active_key_ids are included in the message
        int offset = 5;
        EXPECT_EQ(messages[0][offset + active_key_ids[0]], onCode);
        EXPECT_EQ(messages[0][offset + active_key_ids[1]], onCode);
        EXPECT_EQ(messages[0][offset], 0x01);  // This zero value of the offset should never be overwritten by SetBytesInPacket_RK84, it must be 0x01 per the packet definition
        EXPECT_EQ(messages[0][offset + active_key_ids[3]], offCode); // this won't be set to 0x03 because it's bad
    }

    TEST(KeyboardRK84Attributes, TheAbstractClassHasTheCorrectConstsAvailable) {
        RK84 rk_84;
        AbstractKeyboard* abstr = &rk_84;

        EXPECT_EQ(rk_84.BULK_LED_VALUE_MESSAGES_COUNT, 3);
        EXPECT_EQ(abstr->BULK_LED_VALUE_MESSAGES_COUNT, 3);
        EXPECT_EQ(rk_84.MESSAGE_LENGTH, 65);
        EXPECT_EQ(abstr->MESSAGE_LENGTH, 65);
        EXPECT_EQ(rk_84.keyname_keyid_mappings["tab"], 3);
        EXPECT_EQ(abstr->keyname_keyid_mappings["tab"], 3);
        EXPECT_EQ(rk_84.target_device_path[0], '\\');
        EXPECT_EQ(abstr->target_device_path[0], '\\');
        EXPECT_EQ(rk_84.device_info.vid, 0x258a);
        EXPECT_EQ(abstr->device_info.vid, 0x258a);
        EXPECT_EQ(rk_84.max_key_id, 96);
        EXPECT_EQ(abstr->max_key_id, 96);
    }

}

