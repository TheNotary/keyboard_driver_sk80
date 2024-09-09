#pragma once
#include <iostream>
#include <memory>
#include <basetsd.h>

#include <windows.h>

#include "misc.h"

//#include "keyboard.h"

// Forward declaration allows us to escape that circular dependency trap, though it feels sloppy
#ifndef KEYBOARD_H
#define KEYBOARD_H

class Keyboard;

#endif



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

    // Populates the LED values in the message packets based on the active_key_ids
    virtual void SetBytesInValuePackets(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) = 0;

    // Determines the packets to send and sends them to the usb device
    virtual void SetKeysOnOff(KeyValue key_value, unsigned char* messages, char* active_key_ids, UINT8 n_active_keys) = 0;

    virtual void SetKeysRGB(Keyboard* keyboard_manager, unsigned char r, unsigned char g, unsigned char b) = 0;

    DeviceInfo GetDeviceInfo() const {
        return this->device_info;
    };

    const UINT8 MESSAGE_LENGTH;
    const UINT8 BULK_LED_VALUE_MESSAGES_COUNT;
    const UINT8 max_key_id;
    const char* target_device_path;
    const DeviceInfo device_info;
    HANDLE device_handle = nullptr;
    std::unordered_map<std::string, char> keyname_keyid_mappings;

private:

};
