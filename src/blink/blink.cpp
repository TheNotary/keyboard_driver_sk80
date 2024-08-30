#include <windows.h>
#include <cstdio>
#include "../../include/blink.h"
#include "../../include/print.h"
#include <keyboard.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

extern "C" void HelloDll() {
    printf("Hello from the DLL!\n");
}

extern "C" int BlinkKeys(char* keyIds, int nKeys) {
    PrintBlinkKeysArguments(keyIds, nKeys);

    // Begin a blink 
    Keyboard kbd;

    if (!kbd.Found()) {
        printf("Could not find keyboard");
        return 1;
    }

    kbd.Blink(4, 50);
    kbd.Dispose();
    return 0;
}
