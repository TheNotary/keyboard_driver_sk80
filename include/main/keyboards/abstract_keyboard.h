#pragma once
#include <iostream>
#include <memory>
#include "misc.h"

// TODO: Rename to AbstractKeyboard 
class AbstractKeyboard {
public:
    // Virtual destructor to ensure proper cleanup
    virtual ~AbstractKeyboard() = default;

    virtual void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const = 0;

    struct DeviceInfo {
        short vid;
        short pid;
    };

    DeviceInfo device_mappings;

    KeyNameKeyIdPair keyname_keyid_mappings;

private:


};
