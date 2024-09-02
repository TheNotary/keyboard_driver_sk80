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

    /*char keyIds[] = { 0x01, 0x02, 0x03, 0x0d };

    return test_dll(keyIds, sizeof(keyIds));*/

    //std::vector<std::string> key_names = { "f5", "f6", "f7", "f8", "f9", "f10" };
    std::vector<std::string> key_names = { "f9" };

    return CallDllBlinkKeyNames(key_names);
}
