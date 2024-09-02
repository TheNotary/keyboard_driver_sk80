#include <windows.h>
#include <cstdio>
#include "../../include/blink.h"
#include "../../include/print.h"
#include <keyboard.h>
#include <stdexcept>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")


extern "C" void HelloDll() {
    printf("Hello from the DLL!\n");
}

extern "C" int BlinkKeys(char* keyIds, int nKeys) {
    PrintBlinkKeysArguments(keyIds, nKeys);

    //Keyboard kbd(KeyboardModel::SK80);
    Keyboard kbd(KeyboardModel::RK84);

    printf("pid: %x\n", kbd.GetPid());

    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    return 0;

    kbd.SetActiveKeyIds(keyIds, nKeys);
    //kbd.BlinkActiveKeys(4, 50);
    //kbd.TurnOffActiveKeys(4, 50);

    kbd.TurnOnActiveKeys();

    //kbd.Blink(4, 50);
    kbd.Dispose();
    return 0;
}


extern "C" int TurnOnKeyNames(const std::vector<std::string>& key_names) {
    Keyboard kbd(KeyboardModel::RK84);

    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeys(key_names);

    kbd.TurnOnActiveKeys();

    kbd.Dispose();
    return 0;
}

extern "C" int TurnOnOffNames(const std::vector<std::string>& key_names) {
    Keyboard kbd(KeyboardModel::RK84);
    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeys(key_names);

    kbd.TurnOffActiveKeys();

    kbd.Dispose();
    return 0;
}