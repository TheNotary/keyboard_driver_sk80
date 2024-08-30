#include <iostream>
#include <fmt/core.h>
#include "test_load_blink.h"


int main() {
    fmt::println("Hello fmt, a simple dependency.");

    char keyIds[] = { 0x0d, 0x01 };

    return test_dll(keyIds, sizeof(keyIds));
}
