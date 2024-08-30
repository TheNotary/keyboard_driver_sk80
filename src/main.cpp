#include "../include/usb_functions.h"
#include <windows.h>
#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <setupapi.h>
#include <stdio.h>
#include <hidsdi.h>
#include <locale>
#include <string>
#include <codecvt>
#include "../include/messages.h"
#include "../include/keyboard.h"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

int main() {
    fmt::println("Hello fmt, a simple dependency.");

    Keyboard kbd;
    if (!kbd.Found()) {
        printf("Could not find keyboard");
        return 1;
    }
    kbd.Blink(4, 50);
    kbd.Dispose();

    return 0;
}
