#include "../../include/blink.h"

#include <cstdio>
#include <stdexcept>

#include <windows.h>

#include "../../include/print.h"
#include "keyboard.h"
#include "misc.h"
#include "usb_functions.h"


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

extern "C" {
    void HelloDll() {
        printf("Hello from the DLL!\n");
    }

    int BlinkKeys(char* keyIds, int nKeys) {
        PrintBlinkKeysArguments(keyIds, nKeys);

        //Keyboard kbd(KeyboardModel::SK80);
        Keyboard kbd(KeyboardModel::kRK84);

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

    int TurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char messages_sent[3][65], KeyboardInfo keyboard) {
        return TurnKeyIds(key_ids, n_keys, kOn, messages_sent, keyboard);
    }

    int TurnOnKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard) {
        return TurnKeyIds(key_ids, n_keys, kOn, keyboard);
    }

    int TurnOffKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard) {
        return TurnKeyIds(key_ids, n_keys, kOff, keyboard);
    }

    int TurnOnKeyNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
        return TurnKeyNames(key_names, kOn, keyboard);
    }

    int TurnOnOffNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
        return TurnKeyNames(key_names, kOff, keyboard);
    }

    int ListAvailableKeyboards(KeyboardInfo** out_keyboards) {
        std::vector<KeyboardInfo> keyboards = ListAvailableKeyboards();

        *out_keyboards = (KeyboardInfo*)malloc(keyboards.size() * sizeof(KeyboardInfo));
        if (!(*out_keyboards)) {
            return -1;
        }

        std::memcpy(*out_keyboards, keyboards.data(), keyboards.size() * sizeof(KeyboardInfo));

        return keyboards.size();
    }

    void FreeKeyboards(KeyboardInfo* keyboards) {
        free(keyboards);
    }
}