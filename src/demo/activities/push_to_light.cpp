#include <windows.h>
#include <iostream>
#include <vector>
#include "blink_loader.h"


namespace demo {
using KeyboardInfo = blink::KeyboardInfo;


int PushToLight(KeyboardInfo keyboard) {
    std::cout << "Press F12..." << std::endl;

    while (true) {
        // 0x26 is the virtual key code for the up arrow key

        if (GetAsyncKeyState(VK_F1) & 0x8000) {
            std::cout << "F1 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f1" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F2) & 0x8000) {
            std::cout << "F2 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f2" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F3) & 0x8000) {
            std::cout << "F3 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f3" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F4) & 0x8000) {
            std::cout << "F4 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f4" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            std::cout << "F5 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f5" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F6) & 0x8000) {
            std::cout << "F6 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f6" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F7) & 0x8000) {
            std::cout << "F7 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f7" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            std::cout << "F8 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f8" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            std::cout << "F9 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f9" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F10) & 0x8000) {
            std::cout << "F10 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f10" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F11) & 0x8000) {
            std::cout << "F11 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f11" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_F12) & 0x8000) {
            std::cout << "F12 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f12" };
            CallDllTurnOnKeyNames(key_names, keyboard);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return 0;
        }

        // Optionally, you can include a small delay to prevent the loop from consuming too much CPU
        Sleep(10);
    }

    return 0;
}

}
