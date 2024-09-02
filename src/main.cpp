#include <windows.h>
#include <iostream>
#include <fmt/core.h>
#include "test_load_blink.h"
#include <basetsd.h>
#include <vector>

void printStringsWithPlus(const std::vector<std::string>& strings) {
    for (size_t i = 0; i < strings.size(); ++i) {
        std::cout << strings[i];
        if (i < strings.size() - 1) {
            std::cout << " + ";
        }
    }
    std::cout << std::endl; // End the line after printing all strings
}


int main() {
    fmt::println("Hello fmt, a simple dependency.");

    std::vector<std::string> key_names = { "esc", "~", "f1", "f2", "f3", "f4", 
        "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12" };

    CallDllTurnOnKeyNames(key_names);

    Sleep(1050);

    CallDllTurnOffKeyNames(key_names);

    return 0;
}
