#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
static inline void Sleep(int ms) { usleep(ms * 1000); }
#endif
#include <iostream>
#include <vector>
#include "blink_loader.h"


namespace demo {
using KeyboardInfo = blink::KeyboardInfo;


int PushToLight(KeyboardInfo keyboard) {
#ifdef _WIN32
    std::cout << "Press F1-F12 to light corresponding key, ESC to exit..." << std::endl;

    while (true) {
        if (GetAsyncKeyState(VK_F1) & 0x8000) {
            std::cout << "F1 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f1" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F2) & 0x8000) {
            std::cout << "F2 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f2" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F3) & 0x8000) {
            std::cout << "F3 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f3" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F4) & 0x8000) {
            std::cout << "F4 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f4" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            std::cout << "F5 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f5" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F6) & 0x8000) {
            std::cout << "F6 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f6" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F7) & 0x8000) {
            std::cout << "F7 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f7" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            std::cout << "F8 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f8" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            std::cout << "F9 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f9" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F10) & 0x8000) {
            std::cout << "F10 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f10" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F11) & 0x8000) {
            std::cout << "F11 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f11" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_F12) & 0x8000) {
            std::cout << "F12 key pressed!" << std::endl;
            std::vector<std::string> key_names = { "f12" };
            CallDllTurnOnKeyNames(key_names, keyboard);
            Sleep(200);
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return 0;
        }

        Sleep(10);
    }
#else
    std::cout << "Press 1-9,0,-,= to light F1-F12, q/ESC to exit..." << std::endl;

    // Set terminal to raw mode
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

    // Map characters to F-key names: 1=f1, 2=f2, ..., 9=f9, 0=f10, -=f11, ==f12
    while (true) {
        unsigned char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n == 1) {
            std::string key_name;
            if (ch >= '1' && ch <= '9') {
                key_name = "f" + std::to_string(ch - '0');
            } else if (ch == '0') {
                key_name = "f10";
            } else if (ch == '-') {
                key_name = "f11";
            } else if (ch == '=') {
                key_name = "f12";
            } else if (ch == 'q' || ch == 27) {
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                return 0;
            }

            if (!key_name.empty()) {
                std::cout << key_name << " pressed!" << std::endl;
                std::vector<std::string> key_names = { key_name };
                CallDllTurnOnKeyNames(key_names, keyboard);
                Sleep(200);
            }
        }
        Sleep(10);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    return 0;
}

}
