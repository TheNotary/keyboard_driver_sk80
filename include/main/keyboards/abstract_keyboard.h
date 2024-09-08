#pragma once
#include <iostream>
#include <memory>
#include <basetsd.h>
#include "misc.h"

class AbstractKeyboard {
public:
    struct DeviceInfo {
        short vid;
        short pid;
    };

    AbstractKeyboard(const UINT8 messageLength, const UINT8 bulkLedMessagesCount, const char* target_device_path, DeviceInfo device_info, std::unordered_map<std::string, char> keyname_keyid_mappings)
        : 
        MESSAGE_LENGTH(messageLength), 
        BULK_LED_VALUE_MESSAGES_COUNT(bulkLedMessagesCount),
        target_device_path(target_device_path),
        device_info(device_info),
        keyname_keyid_mappings(keyname_keyid_mappings)
    {}
   
    virtual ~AbstractKeyboard() = default;   // Virtual destructor to ensure proper cleanup

    virtual void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) = 0;

    virtual DeviceInfo GetDeviceInfo() const = 0;

    const UINT8 MESSAGE_LENGTH;
    const UINT8 BULK_LED_VALUE_MESSAGES_COUNT;
    const char* target_device_path;
    DeviceInfo device_info;
    std::unordered_map<std::string, char> keyname_keyid_mappings;

private:


};
