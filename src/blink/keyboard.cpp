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
#include "messages_rk84.h"


std::unordered_map<KeyboardModel, KeyNameKeyIdPair> keyname_keyid_mappings = {
    {
        SK80, {
            {"esc", 0x01},
            {"f1", 0x02},
            {"f2", 0x03},
            {"f3", 0x04},
            {"f4", 0x05},
            {"f5", 0x06},
            {"f6", 0x07},
            {"f7", 0x08},
            {"f8", 0x09},
            {"f9", 0x0a},
            {"f10", 0x0b},
            {"f11", 0x0c},
            {"f12", 0x0d},

            {"tilde", 0x10},
        }
    },
    {
        MK84, {
            {"f12", 0x0d}
        }
    },
    {
        RK84, {
            {"esc", 0x01},
            {"f1", 0x07},
            {"f2", 0x0d},
            {"f3", 0x13},
            {"f4", 0x19},
            {"f5", 0x1f},
            {"f6", 0x25},
            {"f7", 0x2b},
            {"f8", 0x31},
            {"f9", 0x37},
            {"f10", 0x3d},
            {"f11", 0x43},
            {"f12", 0x49},

            {"tilde", 0x02},
        }
    }
};


std::unordered_map<KeyboardModel, KeyValueBytesPair> on_off_mappings = {
    {
        SK80, {
            { kOn, 0xff },
            { kOff, 0x00 }
         }
    },
    {
        RK84, {
            { kOn, 0x07 },
            { kOff, 0x00 }
        }
    },
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
}

bool Keyboard::Found() {
    return this->device_handle != 0;
}

void Keyboard::SetActiveKeys(char* key_ids, UINT8 n_keys) {
    for (int i = 0; i < n_keys; i++) {
        this->active_key_ids[i] = key_ids[i];
    }
    this->active_key_ids[n_keys] = 0x00; // null terminate the key_ids list in case we drop using n_keys
    this->n_active_keys = n_keys;
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
        UINT8 active_key = this->active_key_ids[i];
        // for 0x01, we need to insert into i = 0; offset =

        UINT8 offset = 5 + ((active_key - 1) * 4);

        key_message[0][offset + 0] = this->active_key_ids[i];
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
    std::cout << "SetKeysOnOff" << std::endl;
    if (this->n_active_keys == 0) {
        printf("SetKeysOnOff was called with zero active keys... odd...");
        return;
    }

    char bytesForValue = on_off_mappings[this->keyboard_model][key_value];

    //{
    //    0x0a, 0x03, 0x01, 0x03, 0x7e, 0x01, 0x00, 0x00,  // first key address is after the 0x7e01, escape key
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00
    //},
    //{
    //    0x0a, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00
    //},
    //{
    //    0x0a, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //    0x00
    //}

    //BULK_LED_VALUE_MESSAGES_RK84

    // SendBufferToDevice(this->device_handle, BULK_LED_VALUE_MESSAGES_RK84, BULK_LED_VALUE_MESSAGES_COUNT_RK84, MESSAGE_LENGTH_RK84);
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

void Keyboard::SetupKeyboardModel(KeyboardModel keyboard_model) {
    DeviceInfo device_info = device_mappings[keyboard_model];
    
    this->keyboard_model = keyboard_model;
    this->pid = device_info.pid;
    this->vid = device_info.vid;
}

short Keyboard::GetPid() {
    return this->pid;
}