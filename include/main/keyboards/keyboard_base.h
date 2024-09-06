#pragma once
#include <iostream>
#include <memory>
#include <misc.h>

// TODO: Rename to AbstractKeyboard 
class KeyboardBase {
public:
    // Virtual destructor to ensure proper cleanup
    virtual ~KeyboardBase() = default;

    virtual void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const = 0;
};
