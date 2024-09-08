#include "test_single_key.h"

#include <Windows.h>
#include <iostream>

#include "misc.h"
#include "../blink_loader.h"


namespace demo {

	void TestSingleKey(KeyboardInfo keyboard) {
        std::vector<std::string> key_names = { "esc" };

        demo::CallDllTurnOnKeyNames(key_names, keyboard);
        std::cout << "The key should now be on." << std::endl;
        Sleep(1050);
        demo::CallDllTurnOffKeyNames(key_names, keyboard);
        std::cout << "The key should now be off." << std::endl;
        std::cout << ":)" << std::endl;
    }

}
