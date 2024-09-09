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

    AbstractKeyboard(const UINT8 messageLength, const UINT8 bulkLedMessagesCount, const char* target_device_path, DeviceInfo device_info, std::unordered_map<std::string, char> keyname_keyid_mappings, const UINT8 max_key_id)
        :
        MESSAGE_LENGTH(messageLength),
        BULK_LED_VALUE_MESSAGES_COUNT(bulkLedMessagesCount),
        target_device_path(target_device_path),
        device_info(device_info),
        keyname_keyid_mappings(keyname_keyid_mappings),
        max_key_id(max_key_id)
    {}
   
    virtual ~AbstractKeyboard() = default;   // Virtual destructor to ensure proper cleanup

    // This function will populate the LED values in the message packets based on the active_key_ids
    virtual void SetBytesInValuePackets(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) = 0;

    DeviceInfo GetDeviceInfo() const {
        return this->device_info;
    };

    const UINT8 MESSAGE_LENGTH;
    const UINT8 BULK_LED_VALUE_MESSAGES_COUNT;
    const UINT8 max_key_id;
    const char* target_device_path;
    const DeviceInfo device_info;
    std::unordered_map<std::string, char> keyname_keyid_mappings;

private:

};
