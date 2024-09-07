#include <windows.h>
#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <setupapi.h>
#include <stdio.h>
#include <hidsdi.h>
#include "messages.h"
#include "usb_functions.h"
#include "keyboard.h"
#include "keyboards/rk84/messages_rk84.h"
#include "keyboards/rk84/rk84.h"
#include "keyboards/keyboard_base.h"


std::unordered_map<KeyboardModel, KeyNameKeyIdPair> keyname_keyid_mappings = {
    {
        kSK80, {
            {"esc", 0x01},
            {"f1" , 0x02},
            {"f2" , 0x03},
            {"f3" , 0x04},
            {"f4" , 0x05},
            {"f5" , 0x06},
            {"f6" , 0x07},
            {"f7" , 0x08},
            {"f8" , 0x09},
            {"f9" , 0x0a},
            {"f10", 0x0b},
            {"f11", 0x0c},
            {"f12", 0x0d},

            {"~", 0x10},
        }
    },
    {
        kMK84, {
            {"f12", 0x0d}
        }
    },
    {
        kRK84, {
            {"esc", 1},
            {"f1", 7},
            {"f2", 13},
            {"f3", 19},
            {"f4", 25},
            {"f5", 31},
            {"f6", 37},
            {"f7", 43},
            {"f8", 49},
            {"f9", 55},
            {"f10", 61},
            {"f11", 67},
            {"f12", 73},
            {"print", 79},
            {"pause", 85},
            {"delete", 91},

            {"`", 2},
            {"1", 8},
            {"2", 14},
            {"3", 20},
            {"4", 26},
            {"5", 32},
            {"6", 38},
            {"7", 44},
            {"8", 50},
            {"9", 56},
            {"0", 62},
            {"-", 68},
            {"=", 74},
            {"backspace", 80},
            {"home", 92},

            {"tab", 3},
            {"q", 9},
            {"w", 15},
            {"e", 21},
            {"r", 27},
            {"t", 33},
            {"y", 39},
            {"u", 45},
            {"i", 51},
            {"o", 57},
            {"p", 63},
            {"[", 69},
            {"]", 75},
            {"\\", 81},
            {"end", 93},

            {"caps", 4},
            {"a", 10},
            {"s", 16},
            {"d", 22},
            {"f", 28},
            {"g", 34},
            {"h", 40},
            {"j", 46},
            {"k", 52},
            {"l", 58},
            {";", 64},
            {"'", 70},
            {"enter", 82},
            {"pageup", 94},

            {"lshift", 5},
            {"z", 11},
            {"x", 17},
            {"c", 23},
            {"v", 29},
            {"b", 35},
            {"n", 41},
            {"m", 47},
            {",", 53},
            {".", 59},
            {"/", 65},
            {"rshift", 71},
            {"up", 83},
            {"pagedown", 95},

            {"lctrl", 6},
            {"lmeta", 12},
            {"lalt", 18},
            {"space", 36},
            {"ralt", 54},
            {"fn", 60},
            {"rctrl", 66},
            {"left", 72},
            {"down", 84},
            {"right", 96},
        }
    }
};




Keyboard::Keyboard(KeyboardModel keyboard_model)
{
    this->SetupKeyboardModel(keyboard_model);
    this->AccessDeviceHandle();
}

bool Keyboard::AccessDeviceHandle() {
    this->device_handle = SearchForDevice(this->vid, this->pid);
    //this->device_handle = SearchForDevice("0C45", "8006");
    return true;
}

HANDLE Keyboard::GetDeviceHandle() {
    return this->device_handle;
}

void Keyboard::SetKeyRGB(char key_id, unsigned char r, unsigned char g, unsigned char b) {
    std::cout << "Setting LED" << std::endl;

    SendBufferToDeviceAndGetResp(this->device_handle, TEST_SLIM_HEADER_MESSAGES, 2, MESSAGE_LENGTH);

    TEST_SLIM_MESSAGES[0][54] = r;
    TEST_SLIM_MESSAGES[0][55] = g;
    TEST_SLIM_MESSAGES[0][56] = b;

    SendBufferToDevice(this->device_handle, TEST_SLIM_MESSAGES, 1, MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, END_BULK_UPDATE_MESSAGES, END_BULK_UPDATE_MESSAGE_COUNT, MESSAGE_LENGTH);
}

void Keyboard::Blink(int n, int interval) {
    for (int i = 0; i < n; i++) {
        this->SetKeyRGB(0x0d, 0xff, 0xff, 0xff);
        Sleep(50);
        this->SetKeyRGB(0x0d, 0xff, 0x00, 0x00);
        Sleep(50);
        this->SetKeyRGB(0x0d, 0x00, 0xff, 0x00);
        Sleep(50);
        this->SetKeyRGB(0x0d, 0xff, 0x00, 0x00);
        Sleep(50);
        this->SetKeyRGB(0x0d, 0x00, 0x00, 0xff);
        Sleep(50);
        this->SetKeyRGB(0x0d, 0xff, 0x00, 0x00);
        Sleep(50);
        this->SetKeyRGB(0x0d, 0x00, 0x00, 0x00);
        Sleep(50);
    }
}

void Keyboard::Dispose() {
    CloseHandle(this->device_handle);
    delete this->keyboard_spec;
}

bool Keyboard::Found() {
    return this->device_handle != 0;
}

char Keyboard::GetActiveKeyId(int index) {
    if (index >= 255) {
        std::cerr << "Error GetActiveKeyId: index was greater than the max size "
            << "index: " << index << std::endl;
        return 0;
    }
    return this->active_key_ids[index];
}

void Keyboard::SetActiveKeyId(int index, char key_id) {
    if (index >= 255) {
        std::cerr << "Error SetActiveKeyId: Cannot set a key at index greater than 255.  "
            << "index: " << index << ", key_id: " << key_id << std::endl;
        throw std::invalid_argument("SetActiveKeyId called with index > 255");
    }
    this->active_key_ids[index] = key_id;
}

void Keyboard::SetActiveKeyIds(char* key_ids, UINT8 n_keys) {
    if (n_keys >= 255) {
        std::cerr << "Error SetActiveKeyIds: Tried to set over 255 keys!  "
            << "n_keys: " << n_keys << std::endl;
        return;
    }
    for (int i = 0; i < n_keys; i++) {
        this->SetActiveKeyId(i, key_ids[i]);
    }
    this->SetActiveKeyId(n_keys, 0x00); // null terminate the key_ids list in case we drop using n_keys
    this->n_active_keys = n_keys;
}

void Keyboard::SetActiveKeys(const std::vector<std::string>& key_names) {
    if (key_names.size() >= 255) {
        std::cout << "Error: list of key_names to set was too great" << std::endl;
        throw std::invalid_argument("key_names had too many entries.");
    }

    for (UINT8 i = 0; i < key_names.size(); ++i) {
        std::cout << "Index: " << i << ", Value: " << key_names[i] << std::endl;
        std::string key_name = key_names[i];
        char key_id = keyname_keyid_mappings[this->keyboard_model][key_name];
        if (key_id == 0) {
            std::cout << "Error: could not lookup up key " 
                << "[ " << key_name << " ]"
                << " from keyname_keyid_mappings." << std::endl;
            throw std::invalid_argument(
                "Error: could not lookup up key from keyname_keyid_mappings");
        }
        this->SetActiveKeyId(i, key_id);
    }
    this->n_active_keys = (UINT8)key_names.size();
}

void Keyboard::SetKeysRGB(unsigned char r, unsigned char g, unsigned char b) {
    if (this->n_active_keys == 0) {
        printf("SetKeysRGB was called with zero active keys... odd...");
        return;
    }

    /*char key_message[65] = {
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
        0x0c, 0x00, 0x00, 0x00, 0x0d, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };*/

    unsigned char key_message[1][65] = { 0x00 };

    UINT8 n_packets = ((this->n_active_keys - 1u) / 15u) + 1u;

    // What about this, for every active key, let's pull up the index it should be in the first packet
    // and send it...
    for (int i = 0; i < this->n_active_keys; i++) {
        UINT8 active_key = this->GetActiveKeyId(i);
        // for 0x01, we need to insert into i = 0; offset = 

        UINT8 offset = 5 + ((active_key - 1) * 4);

        key_message[0][offset + 0] = this->GetActiveKeyId(i);
        key_message[0][offset + 1] = r;
        key_message[0][offset + 2] = g;
        key_message[0][offset + 3] = b;
    }

    //// we can only have 15 colors per page
    //for (int i = 0; i < this->n_active_keys; i++) {
    //    UINT8 offset = 5 + (i * 4);

    //    key_message[0][offset + 0] = this->active_key_ids[i];
    //    key_message[0][offset + 1] = r;
    //    key_message[0][offset + 2] = g;
    //    key_message[0][offset + 3] = b;
    //}



    printf("Sending RGB message: \n");
    printf("0x00");
    for (int i = 1; i < MESSAGE_LENGTH; i++) {
        if ((i-1) % 8 == 0)
            printf("\n");
        printf("0x%02x ", key_message[0][i]);
    }
    printf("\n");

    SendBufferToDeviceAndGetResp(this->device_handle, TEST_SLIM_HEADER_MESSAGES, 2, MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, key_message, 1, MESSAGE_LENGTH);

    unsigned char msg2[1][65] = { // 2
        0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x13, 0xff, 0x00, 0x00,
        0x14, 0xff, 0x00, 0x00, 0x15, 0xff, 0x00, 0x00,
        0x16, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
        0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
        0x1a, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00,
        0x1c, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
        0x1e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00
    };

    SendBufferToDevice(this->device_handle, msg2, 1, MESSAGE_LENGTH);

    //SendBufferToDevice(this->device_handle, TEST_SLIM_MESSAGES, 1, MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, END_BULK_UPDATE_MESSAGES, END_BULK_UPDATE_MESSAGE_COUNT, MESSAGE_LENGTH);
}


void Keyboard::SetKeysOnOff(KeyValue key_value) {
    unsigned char messages[BULK_LED_VALUE_MESSAGES_COUNT_RK84][MESSAGE_LENGTH_RK84];
    this->SetKeysOnOff(key_value, messages);
}

void Keyboard::SetKeysOnOff(KeyValue key_value, unsigned char messages_sent[3][65]) {
    if (this->n_active_keys == 0) {
        printf("SetKeysOnOff was called with zero active keys... odd...skipping");
        return;
    }

    this->keyboard_spec->SetBytesInPacket(*messages_sent, key_value, this->active_key_ids, this->n_active_keys);

    // PrintMessagesInBuffer(messages_sent, BULK_LED_VALUE_MESSAGES_COUNT_RK84, MESSAGE_LENGTH_RK84);

    SendBufferToDevice(this->device_handle, messages_sent, BULK_LED_VALUE_MESSAGES_COUNT_RK84, MESSAGE_LENGTH_RK84);
}

void Keyboard::BlinkActiveKeys(int n, int interval) {
    for (int i = 0; i < n; i++) {
        this->SetKeysRGB(0xff, 0xff, 0xff);
        Sleep(interval);
        this->SetKeysRGB(0xff, 0x00, 0x00);
        Sleep(interval);
        this->SetKeysRGB(0x00, 0xff, 0x00);
        Sleep(interval);
        this->SetKeysRGB(0xff, 0x00, 0x00);
        Sleep(interval);
        this->SetKeysRGB(0x00, 0x00, 0xff);
        Sleep(interval);
        this->SetKeysRGB(0xff, 0x00, 0x00);
        Sleep(interval);
        this->SetKeysRGB(0x00, 0x00, 0x00);
        Sleep(interval);
    }
}

void Keyboard::TurnOnActiveKeys() {
    this->SetKeysOnOff(kOn);
}

void Keyboard::TurnOnActiveKeys(unsigned char messages_sent[3][65]) {
    this->SetKeysOnOff(kOn, messages_sent);
}

void Keyboard::TurnOffActiveKeys() {
    this->SetKeysOnOff(kOff);
}

void Keyboard::SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const {
    this->keyboard_spec->SetBytesInPacket(messages, key_value, active_key_ids, n_active_keys);
}

void Keyboard::SetupKeyboardModel(KeyboardModel keyboard_model) {
    KeyboardBase::DeviceInfo device_info = this->keyboard_spec->device_mappings;
    
    this->keyboard_model = keyboard_model;
    this->pid = device_info.pid;
    this->vid = device_info.vid;

    // Could use the unique_ptr function to use a smart pointer, but since I already have a Dispose function, 
    // using standard heap initialization is fine.
    //std::unique_ptr<KeyboardBase> my_base = std::make_unique<RK84>();
    // this->keyboard_spec = new (this->keyboard_spec) RK84();

    switch (keyboard_model) {
    case (kRK84):
        this->keyboard_spec = new RK84();
        break;
    default:
        this->keyboard_spec = nullptr;
        printf("this keyboard_spec not implemented yet");
        throw("keyboard_spec not implemented yet");
    }
}

short Keyboard::GetPid() {
    return this->pid;
}
