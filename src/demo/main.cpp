#include <iostream>
#include <basetsd.h>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include <windows.h>
#include <fmt/core.h>

#include "choose_keyboard.h"
#include "choose_activity.h"
#include "blink_loader.h"
#include "misc.h"
#include "activities/push_to_light.h"
#include "activities/cycle_keyids.h"
#include "activities/test_single_key.h"


int main() {

    KeyboardInfo keyboard = demo::ChooseKeyboard();
    if (keyboard.display_name[0] == 0x00) {
        std::cout << "Exiting, no valid keyboard selected" << std::endl;
        return 0;
    }

    int activity = demo::ChooseActivity();  // 1 == CycleKeyIds

    if (activity == 1) {
        demo::CycleKeyIds(keyboard);
        return 0;
    }
    if (activity == 2) {
        demo::PushToLight(keyboard);
        return 0;
    }
    if (activity == 3) {
        demo::TestSingleKey(keyboard);
        return 0;
    }

    std::cout << "Invalid choice." << std::endl;
    return 0;


    
    
}
