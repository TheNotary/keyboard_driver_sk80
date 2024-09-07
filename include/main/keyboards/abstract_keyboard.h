#pragma once
#include <iostream>
#include <memory>
#include <basetsd.h>
#include "misc.h"

class AbstractKeyboard {
public:
    AbstractKeyboard(const UINT8 messageLength, const UINT8 bulkLedMessagesCount)
        : MESSAGE_LENGTH(messageLength), BULK_LED_VALUE_MESSAGES_COUNT(bulkLedMessagesCount) {}

    // Virtual destructor to ensure proper cleanup
    virtual ~AbstractKeyboard() = default;

    virtual void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const = 0;

    struct DeviceInfo {
        short vid;
        short pid;
    };

    DeviceInfo device_mappings;

    KeyNameKeyIdPair keyname_keyid_mappings;

    const UINT8 MESSAGE_LENGTH;
    const UINT8 BULK_LED_VALUE_MESSAGES_COUNT;

private:


};
