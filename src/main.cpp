#include <windows.h>
#include <iostream>
#include <fmt/core.h>
#include <basetsd.h>
#include <vector>
#include <unordered_map>
#include "test_load_blink.h"
#include "push_to_light.h"
#include "cycle_keyids.h"


int main() {

    // std::vector<KeyboardModel> = CheckAvailableKeyboards();

    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };

    std::cout << "Length: " << arr.size() << std::endl;

    return 0;



    CycleKeyIds();
    return 0;

    //push_to_light();
    //return 0;

    fmt::println("Hello fmt, a simple dependency.");

    std::vector<std::string> key_names = { "esc", "~", "f1", "f2", "f3", "f4",
        "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" };

    CallDllTurnOnKeyNames(key_names);
    Sleep(1050);
    CallDllTurnOffKeyNames(key_names);

    return 0;
}
