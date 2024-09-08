#include <iostream>
#include <basetsd.h>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include <windows.h>
#include <fmt/core.h>

#include "choose_keyboard.h"
#include "blink_loader.h"
#include "activities/push_to_light.h"
#include "activities/cycle_keyids.h"
#include "misc.h"


int main() {

    KeyboardInfo keyboard = demo::ChooseKeyboard();
    if (keyboard.display_name[0] == 0x00) {
        return 0;
    }

    demo::CycleKeyIds(keyboard);
    return 0;

    //push_to_light();
    //return 0;

    /*fmt::println("Hello fmt, a simple dependency.");

    std::vector<std::string> key_names = { "esc", "~", "f1", "f2", "f3", "f4",
        "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" };

    demo::CallDllTurnOnKeyNames(key_names);
    Sleep(1050);
    demo::CallDllTurnOffKeyNames(key_names);

    return 0;*/
}
