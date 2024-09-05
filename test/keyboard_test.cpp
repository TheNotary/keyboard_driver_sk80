#include <gtest/gtest.h>
#include "../include/main/keyboard.h"

namespace KeyboardTest
{
    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 1, returns the message_index 
    // to be 0 (the key is transmitted within the first message) and slot 9 of that message.
    TEST(KeyboardTest, KeyInFirstPacket) {
        Keyboard keyboard(KeyboardModel::RK84);
        int n_keys_in_first_packet = 57;
        int keyId = 1;

        // Destructuring using structured bindings
        auto [message_index, slot] = GetMessageIndexAndKeycodeOffsetForKeyId_RK84(keyId);
        // TwoUINT8s result = keyboard.GetMessageIndexAndSlotForKeyId(1, n_keys_in_first_packet);

        EXPECT_EQ(message_index, 0);
        EXPECT_EQ(slot, 6);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 15, returns the message_index 
    // to be 0 (the key is transmitted within the first message) and slot 23 of that message
    TEST(KeyboardTest, KeyInSecondPacket) {
        Keyboard keyboard(KeyboardModel::RK84);
        int n_keys_in_first_packet = 57;  // wait... 59 are in the first packet though... how is this not a bug?
        int keyId = 2;
        auto [message_index, slot] = GetMessageIndexAndKeycodeOffsetForKeyId_RK84(keyId);

        EXPECT_EQ(message_index, 0);
        EXPECT_EQ(slot, 7);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 64, returns the message_index 
    // to be 1 (the key is transmitted within the second message) and slot 10 of that message
    TEST(KeyboardTest, KeyId60IsOnSecondPage) {
        Keyboard keyboard(KeyboardModel::RK84);
        int n_keys_in_first_packet = 57;
        int keyId = 60;
        auto [message_index, slot] = GetMessageIndexAndKeycodeOffsetForKeyId_RK84(keyId);

        EXPECT_EQ(message_index, 1);
        EXPECT_EQ(slot, 3);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 65, returns the message_index 
    // to be 1 (the key is transmitted within the second message) and slot 11 of that message
    TEST(KeyboardTest, KeyId61IsOnSecondPage) {
        Keyboard keyboard(KeyboardModel::RK84);
        int n_keys_in_first_packet = 57;
        int keyId = 61;
        auto [message_index, slot] = GetMessageIndexAndKeycodeOffsetForKeyId_RK84(keyId);

        EXPECT_EQ(message_index, 1);
        EXPECT_EQ(slot, 4);
    }
    
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}