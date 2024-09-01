#include <iostream>
#include <fmt/core.h>
#include "test_load_blink.h"
#include <basetsd.h>

int main() {
    fmt::println("Hello fmt, a simple dependency.");

    char keyIds[] = { 0x01, 0x02, 0x03, 0x0d };

    return test_dll(keyIds, sizeof(keyIds));
}
