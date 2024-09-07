#include <windows.h>
#include <iostream>
#include <fmt/core.h>
#include <basetsd.h>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include "test_load_blink.h"
#include "push_to_light.h"
#include "cycle_keyids.h"
#include "misc.h"


KeyboardInfo ChooseKeyboard() {
    std::vector<KeyboardInfo> available_keebs = _ListAvailableKeyboards();

    std::cout << "Keyboards Available:" << std::endl;
    int i;
    for (i = 0; i < available_keebs.size(); i++) {
        KeyboardInfo keeb = available_keebs[i];
        std::cout << i + 1 << ". " << keeb.display_name 
            << " [" << std::hex << keeb.vid << ":" << keeb.pid << "]"
            << std::endl;
    }

    // TODO: Test if zero available
    if (i == 0) {
        printf("Error:  No keyboard detected!");
        return {};
    }

    std::cout << std::endl << "keyboard [1-" << i << "]:" << std::endl;

    std::string line;
    std::getline(std::cin, line);

    
    int choice;

    try {
        choice = std::stoi(line);
    }
    catch (...) {
        //catch (std::invalid_argument & err) {
        printf("Error: Invalid input, specify a number.\n");
        return {};
    }

    if (choice > i || choice < 0) {
        printf("Error: The number specified isn't a valid choice.\n");
        return {};
    }


    return available_keebs[choice - 1];
}

int main() {

    KeyboardInfo keyboard = ChooseKeyboard();
    if (keyboard.display_name[0] == 0x00) {
        return 0;
    }
    
    CycleKeyIds(keyboard);
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
