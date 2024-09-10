#include "blink.h"

#include <cstdio>
#include <stdexcept>

#include <windows.h>

#include "print.h"
#include "keyboard.h"
#include "misc.h"
#include "usb_functions.h"

namespace blinkdll {
using KeyboardInfo = blink::KeyboardInfo;
using KeyValue = blink::KeyValue;
using Keyboard = blink::Keyboard;


int TurnKeyNames(const std::vector<std::string>& key_names, KeyValue onOrOff, KeyboardInfo keyboard) {
    blink::Keyboard kbd(keyboard.keyboard_model);

    if (!kbd.ConnectToDevice()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeys(key_names);

    if (onOrOff == blink::kOn) {
        kbd.TurnOnActiveKeys();
    }
    else {
        kbd.TurnOffActiveKeys();
    }

    return 0;
}

int TurnKeyIds(char* key_ids, UINT8 n_keys, KeyValue onOrOff, KeyboardInfo keyboard, unsigned char* messages_sent = nullptr) {
    Keyboard kbd(keyboard.keyboard_model);

    if (!kbd.ConnectToDevice()) {
        printf("Could not find keyboard\n");
        return 1;
    }

    kbd.SetActiveKeyIds(key_ids, n_keys);

    if (onOrOff == blink::kOn) {
        if (messages_sent)
            kbd.TurnOnActiveKeys(messages_sent);
        else
            kbd.TurnOnActiveKeys();
    }
    else {
        kbd.TurnOffActiveKeys();
    }

    return 0;
}


extern "C" {

    void PrintMessagesInBufferA(const unsigned char* buffer, size_t message_count, size_t message_length) {
        blink::PrintMessagesInBuffer(buffer, message_count, message_length);
    }

    int BlinkKeys(char* keyIds, int nKeys) {
        blink::PrintBlinkKeysArguments(keyIds, nKeys);

        //Keyboard kbd(KeyboardModel::SK80);
        Keyboard kbd(blink::KeyboardModel::kRK84);

        if (!kbd.ConnectToDevice()) {
            printf("Could not find keyboard\n");
            return 1;
        }

        return 0;

        kbd.SetActiveKeyIds(keyIds, nKeys);
        //kbd.BlinkActiveKeys(4, 50);
        //kbd.TurnOffActiveKeys(4, 50);

        kbd.TurnOnActiveKeys();

        //kbd.Blink(4, 50);
        return 0;
    }

    int TurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char* messages_sent, KeyboardInfo keyboard) {
        return TurnKeyIds(key_ids, n_keys, blink::kOn, keyboard, messages_sent);
    }

    int TurnOnKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard) {
        return TurnKeyIds(key_ids, n_keys, blink::kOn, keyboard);
    }

    int TurnOffKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard) {
        return TurnKeyIds(key_ids, n_keys, blink::kOff, keyboard);
    }

    int TurnOnKeyNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
        return TurnKeyNames(key_names, blink::kOn, keyboard);
    }

    int TurnOnOffNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
        return TurnKeyNames(key_names, blink::kOff, keyboard);
    }

    int ListAvailableKeyboards(KeyboardInfo** out_keyboards) {
        std::vector<KeyboardInfo> keyboards = blink::ListAvailableKeyboards();

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


}
