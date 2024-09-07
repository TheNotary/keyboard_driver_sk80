#pragma once
#include <basetsd.h>
#include <unordered_map>
#include <string>


enum KeyboardModel { kSK80, kMK84, kRK84 };
enum KeyValue { kOn, kOff };

struct TwoUINT8s {
    UINT8 first;
    UINT8 second;
};

// Setup a mapping between, say "f12" and 0x0d (the ID for the key as known by the hardware)
using KeyNameKeyIdPair = std::unordered_map<std::string, char>;
extern std::unordered_map<KeyboardModel, KeyNameKeyIdPair> keyname_keyid_mappings;

using KeyValueBytesPair = std::unordered_map<KeyValue, char>;
extern std::unordered_map<KeyboardModel, KeyValueBytesPair> on_off_mappings;
