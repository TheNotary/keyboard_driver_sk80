#include <gtest/gtest.h>
#include <vector>
#include "../include/main/keyboard.h"
#include "keyboards/rk84/rk84.h"
#include "misc.h"

// TODO: move to separate file for test_demo
#include "../src/demo/blink_loader.h"


namespace KeyboardTest
{
    
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
