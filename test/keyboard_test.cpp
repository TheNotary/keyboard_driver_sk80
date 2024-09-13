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
        int keyId = 1;

        // Destructuring using structured bindings
        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);
        // TwoUINT8s result = keyboard.GetMessageIndexAndSlotForKeyId(1, n_keys_in_first_packet);

        EXPECT_EQ(message_index, 0);
        EXPECT_EQ(slot, 6);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 15, returns the message_index
    // to be 0 (the key is transmitted within the first message) and slot 23 of that message
    TEST(KeyboardTest, KeyInSecondPacket) {
        int keyId = 2;

        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);

        EXPECT_EQ(message_index, 0);
        EXPECT_EQ(slot, 7);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 64, returns the message_index
    // to be 1 (the key is transmitted within the second message) and slot 10 of that message
    TEST(KeyboardTest, KeyId60IsOnSecondPage) {
        int keyId = 60;

        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);

        EXPECT_EQ(message_index, 1);
        EXPECT_EQ(slot, 3);
    }

    // Test keyboard::GetMessageIndexAndSlotForKeyId for keyId 65, returns the message_index
    // to be 1 (the key is transmitted within the second message) and slot 11 of that message
    TEST(KeyboardTest, KeyId61IsOnSecondPage) {
        int keyId = 61;
        auto [message_index, slot] = internal::GetMessageIndexAndKeycodeOffsetForKeyId(keyId);

        EXPECT_EQ(message_index, 1);
        EXPECT_EQ(slot, 4);
    }

    // Mock this or delete it, it's actually listing keyboards...
    // Actually I like this test alot.  If a known keyboard isn't attached, just comment out the last assertion 
    TEST(KeyboardTest, ListAvailableKeyboardsReturnsAListOfBoards) {
        std::vector<blink::KeyboardInfo> list = demo::ListAvailableKeyboards();

        bool was_reasonable = false;

        if (list.size() == 0 || list.size() == 1)
            was_reasonable = true;

        EXPECT_TRUE(was_reasonable);
        
        // Only do this if an KEYBOARD_ATTACHED is set to 1
        #pragma warning(disable: 4996) // disables a lint complaint, tests can call getenv without it being a risk in deployment
        const char* keyboardAttachedEnv = std::getenv("KEYBOARD_ATTACHED");
        if (keyboardAttachedEnv != nullptr && std::string(keyboardAttachedEnv) == "1")
            EXPECT_EQ(list.size(), 1);
    }

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
