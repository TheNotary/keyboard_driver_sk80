#pragma once
#include <basetsd.h>
#include <unordered_map>
#include <string>


enum KeyboardModel { kSK80, kMK84, kRK84 };

struct KeyboardInfo {
    KeyboardModel keyboard_model;
    short vid;
    short pid;
    char display_name[25];
    UINT8 max_key_id;
    UINT8 MESSAGE_LENGTH;
    UINT8 BULK_LED_VALUE_MESSAGES_COUNT;

    bool operator==(const KeyboardInfo& other) const {
        return (keyboard_model == other.keyboard_model &&
            vid == other.vid &&
            pid == other.pid &&
            std::strncmp(display_name, other.display_name, 25) == 0);
    }
};

struct TwoUINT8s {
    UINT8 first;
    UINT8 second;
};

enum KeyValue { kOn, kOff };

// Setup a mapping between, say "f12" and 0x0d (the ID for the key as known by the hardware)
using KeyNameKeyIdPair = std::unordered_map<std::string, char>;
extern std::unordered_map<KeyboardModel, KeyNameKeyIdPair> keyname_keyid_mappings;

using KeyValueBytesPair = std::unordered_map<KeyValue, char>;
extern std::unordered_map<KeyboardModel, KeyValueBytesPair> on_off_mappings;


namespace settings {

    const bool print_packets = false;

}