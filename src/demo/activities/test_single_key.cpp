#include "test_single_key.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
static inline void Sleep(int ms) { usleep(ms * 1000); }
#endif
#include <iostream>

#include "misc.h"
#include "../blink_loader.h"


namespace demo {

	void TestSingleKey(blink::KeyboardInfo keyboard) {
        std::vector<std::string> key_names = { "esc" };

        demo::CallDllTurnOnKeyNames(key_names, keyboard);
        std::cout << "The key should now be on." << std::endl;
        Sleep(1050);
        demo::CallDllTurnOffKeyNames(key_names, keyboard);
        std::cout << "The key should now be off." << std::endl;
        std::cout << ":)" << std::endl;
    }

}
