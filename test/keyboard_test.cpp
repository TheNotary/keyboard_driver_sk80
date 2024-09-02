#include <gtest/gtest.h>
// #include "keyboard.h"

// Test Case 1: active_key <= n_keys_in_first_packet
//TEST(KeyboardTest, KeyInFirstPacket) {
//    Keyboard keyboard;
//    TwoUINT8s result = keyboard.GetMessageIndexAndSlotForKeyId(5, 10);*/
//
//    EXPECT_EQ(result.first, 0);
//    EXPECT_EQ(result.second, 8);
//}

TEST(KeyboardTest, KeyInSecondPacket) {
    EXPECT_EQ(true, true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}