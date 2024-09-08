#include "choose_keyboard.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include "misc.h"
#include "blink_loader.h"


namespace demo {

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


}
