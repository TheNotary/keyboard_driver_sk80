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

    Keyboard kbd(KeyboardModel::SK80);

    printf("pid: %x\n", kbd.GetPid());

    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    return 0;

    kbd.SetActiveKeys(keyIds, nKeys);
    kbd.BlinkActiveKeys(4, 50);

    //kbd.Blink(4, 50);
    kbd.Dispose();
    return 0;
}
