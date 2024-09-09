#include <gtest/gtest.h>
#include <vector>
#include "../include/main/keyboard.h"
#include "keyboards/rk84/rk84.h"
#include "misc.h"

// TODO: move to separate file for test_demo
#include "../src/demo/blink_loader.h"



namespace KeyboardTest
{
    using namespace rk84;

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 1, returns the message_index
    // to be 0 (the key is transmitted within the first message) and slot 9 of that message.
    TEST(KeyboardTest, KeyInFirstPacket) {
        Keyboard keyboard(KeyboardModel::kRK84);
        int n_keys_in_first_packet = 57;
        int keyId = 1;

        RK84 rk84;

        // Destructuring using structured bindings
        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);
        // TwoUINT8s result = keyboard.GetMessageIndexAndSlotForKeyId(1, n_keys_in_first_packet);

        EXPECT_EQ(message_index, 0);
        EXPECT_EQ(slot, 6);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 15, returns the message_index
    // to be 0 (the key is transmitted within the first message) and slot 23 of that message
    TEST(KeyboardTest, KeyInSecondPacket) {
        Keyboard keyboard(KeyboardModel::kRK84);
        int n_keys_in_first_packet = 57;  // wait... 59 are in the first packet though... how is this not a bug?
        int keyId = 2;
        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);

        EXPECT_EQ(message_index, 0);
        EXPECT_EQ(slot, 7);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 64, returns the message_index
    // to be 1 (the key is transmitted within the second message) and slot 10 of that message
    TEST(KeyboardTest, KeyId60IsOnSecondPage) {
        Keyboard keyboard(KeyboardModel::kRK84);
        int n_keys_in_first_packet = 57;
        int keyId = 60;
        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);

        EXPECT_EQ(message_index, 1);
        EXPECT_EQ(slot, 3);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 65, returns the message_index
    // to be 1 (the key is transmitted within the second message) and slot 11 of that message
    TEST(KeyboardTest, KeyId61IsOnSecondPage) {
        Keyboard keyboard(KeyboardModel::kRK84);
        int n_keys_in_first_packet = 57;
        int keyId = 61;
        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);

        EXPECT_EQ(message_index, 1);
        EXPECT_EQ(slot, 4);
    }

    // Mock this or delete it, it's actually listing keyboards...
    // Actually I like this test alot.  If a known keyboard isn't attached, just comment out the last assertion 
    TEST(KeyboardTest, ListAvailableKeyboardsReturnsAListOfBoards) {
        std::vector<KeyboardInfo> list = demo::_ListAvailableKeyboards();

        bool was_reasonable = false;

        if (list.size() == 0 || list.size() == 1)
            was_reasonable = true;

        EXPECT_TRUE(was_reasonable);
        EXPECT_EQ(list.size(), 1);
    }

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
