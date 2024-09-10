#include "choose_keyboard.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include "util.h"
#include "misc.h"
#include "blink_loader.h"


namespace demo {

blink::KeyboardInfo ChooseKeyboard() {
    std::vector<blink::KeyboardInfo> available_keebs = demo::ListAvailableKeyboards();

    std::cout << "Keyboards Available:" << std::endl;
    int i;
    int n_available_keebs = available_keebs.size();
    for (i = 0; i < n_available_keebs; i++) {
        blink::KeyboardInfo keeb = available_keebs[i];
        std::cout << i + 1 << ". " << keeb.display_name
            << " [" << std::hex << keeb.vid << ":" << keeb.pid << "]"
            << std::endl;
    }

    if (i == 1) {
        std::cout << "Only one keyboard available, choosing it." << std::endl << std::endl;
        return available_keebs[0];
    }

    if (i == 0) {
        printf("Error:  No keyboard detected!");
        return {};
    }

    std::cout << std::endl << "keyboard [1-" << i << "]:" << std::endl;

    int choice = ParseNumericChoiceFromStdin();

    if (choice > i || choice < 0) {
        printf("Error: The number specified isn't a valid choice.\n");
        return {};
    }


    return available_keebs[choice - 1];
}


}
