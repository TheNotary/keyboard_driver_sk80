#include <windows.h>
#include <cstdio>
#include "../../include/blink.h"
#include "../../include/print.h"
#include "keyboard.h"
#include "misc.h"
#include "usb_functions.h"
#include <stdexcept>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")


extern "C" void HelloDll() {
    printf("Hello from the DLL!\n");
}

extern "C" int BlinkKeys(char* keyIds, int nKeys) {
    PrintBlinkKeysArguments(keyIds, nKeys);

    //Keyboard kbd(KeyboardModel::SK80);
    Keyboard kbd(KeyboardModel::kRK84);

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

int TurnKeyNames(const std::vector<std::string>& key_names, KeyValue onOrOff, KeyboardInfo keyboard) {
    Keyboard kbd(keyboard.keyboard_model);

    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeys(key_names);

    if (onOrOff == kOn) {
        kbd.TurnOnActiveKeys();
    }
    else {
        kbd.TurnOffActiveKeys();
    }

    kbd.Dispose();
    return 0;
}

int TurnKeyIds(char* key_ids, UINT8 n_keys, KeyValue onOrOff, unsigned char messages_sent[3][65], KeyboardInfo keyboard) {
    Keyboard kbd(keyboard.keyboard_model);

    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeyIds(key_ids, n_keys);

    if (onOrOff == kOn) {
        kbd.TurnOnActiveKeys(*messages_sent);
    }
    else {
        kbd.TurnOffActiveKeys();
    }

    kbd.Dispose();
    return 0;
}

int TurnKeyIds(char* key_ids, UINT8 n_keys, KeyValue onOrOff, KeyboardInfo keyboard) {
    Keyboard kbd(keyboard.keyboard_model);

    if (!kbd.Found()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeyIds(key_ids, n_keys);

    if (onOrOff == kOn) {
        kbd.TurnOnActiveKeys();
    }
    else {
        kbd.TurnOffActiveKeys();
    }

    kbd.Dispose();
    return 0;
}

extern "C" int TurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char messages_sent[3][65], KeyboardInfo keyboard) {
    return TurnKeyIds(key_ids, n_keys, kOn, messages_sent, keyboard);
}

extern "C" int TurnOnKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard) {
    return TurnKeyIds(key_ids, n_keys, kOn, keyboard);
}

extern "C" int TurnOffKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard) {
    return TurnKeyIds(key_ids, n_keys, kOff, keyboard);
}

extern "C" int TurnOnKeyNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
    return TurnKeyNames(key_names, kOn, keyboard);
}

extern "C" int TurnOnOffNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
    return TurnKeyNames(key_names, kOff, keyboard);
}

extern "C" int ListAvailableKeyboards(KeyboardInfo** out_keyboards) {
    std::vector<KeyboardInfo> keyboards = ListAvailableKeyboards();

    *out_keyboards = (KeyboardInfo*)malloc(keyboards.size() * sizeof(KeyboardInfo));
    if (!(*out_keyboards)) {
        return -1;
    }

    std::memcpy(*out_keyboards, keyboards.data(), keyboards.size() * sizeof(KeyboardInfo));

    return keyboards.size();
}

extern "C" void FreeKeyboards(KeyboardInfo* keyboards) {
    free(keyboards);
}