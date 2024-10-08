#include "keyboard.h"

#include <iostream>
#include <vector>

#include "messages.h"
#include "usb_functions.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/keyboard_spec_factory.h"

namespace blink {


Keyboard::Keyboard(KeyboardModel keyboard_model)
{
    this->SetupKeyboardModel(keyboard_model);
}

void Keyboard::SetupKeyboardModel(KeyboardModel keyboard_model) {
    KeyboardSpecFactory kf;
    this->keyboard_spec = kf.CreateKeyboardSpec(keyboard_model, this);

    if (this->keyboard_spec == nullptr) {
        printf("this keyboard_spec not implemented yet");
        throw("keyboard_spec not implemented yet");
    }

    AbstractKeyboard::DeviceInfo device_info = this->keyboard_spec->GetDeviceInfo();
    this->pid = device_info.pid;
    this->vid = device_info.vid;
    this->keyboard_model = keyboard_model;
}

bool Keyboard::ConnectToDevice() {
    this->device_handle = SearchForDevice(this->vid, this->pid, this->keyboard_spec->target_device_path);
    this->keyboard_spec->device_handle = this->device_handle;
    return true;
}

// This method is automatically called when the class goes out of scope.  It will...
//   - Close the open handle to the keyboard device
//   - Clean up the keyboard_spec object from the heap
void Keyboard::Dispose() {
    if (this->device_handle)
        CloseHandle(this->device_handle);
    delete this->keyboard_spec;
}

// Returns true if the Keyboard's device_handle was retrieved successfully
bool Keyboard::Found() {
    return !this->device_handle;
}

void Keyboard::SetActiveKeys(const std::vector<std::string>& key_names) {
    if (key_names.size() >= 255) {
        std::cout << "Error: list of key_names to set was too great" << std::endl;
        throw std::invalid_argument("key_names had too many entries.");
    }

    for (UINT8 i = 0; i < key_names.size(); ++i) {
        std::string key_name = key_names[i];
        char key_id = this->keyboard_spec->keyname_keyid_mappings[key_name];
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

void Keyboard::SetActiveKeyIds(const char* key_ids, UINT8 n_keys) {
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

void Keyboard::SetKeysOnOff(KeyValue key_value) {
    unsigned char* messages = new unsigned char[
        this->keyboard_spec->BULK_LED_VALUE_MESSAGES_COUNT * 
        this->keyboard_spec->MESSAGE_LENGTH];

    this->SetKeysOnOff(key_value, messages);
    delete[] messages;
}

void Keyboard::SetKeysOnOff(KeyValue key_value, unsigned char* messages) {
    this->keyboard_spec->SetKeysOnOff(key_value, messages);
}

void Keyboard::TurnOnActiveKeys() {
    this->SetKeysOnOff(kOn);
}

void Keyboard::TurnOnActiveKeys(unsigned char* messages_sent) {
    this->SetKeysOnOff(kOn, messages_sent);
}

void Keyboard::TurnOffActiveKeys() {
    this->SetKeysOnOff(kOff);
}


// TODO: Re-write this to accept a string, look up key_id from keyboard_spec
void Keyboard::Blink(int n, int interval) {
    // this->keyboard_spec->blink();
    /*for (int i = 0; i < n; i++) {
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
    }*/
}

// TODO: Rewrite this to accept a string, look up key_id from keyboard_spec
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

// TODO: Specific to SK80
void Keyboard::SetKeysRGB(unsigned char r, unsigned char b, unsigned char g) {
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

    //// we can only have 15 colors per page
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

    printf("Sending RGB message: \n");
    printf("0x00");
    for (int i = 1; i < MESSAGE_LENGTH; i++) {
        if ((i - 1) % 8 == 0)
            printf("\n");
        printf("0x%02x ", key_message[0][i]);
    }
    printf("\n");

    SendBufferToDeviceAndGetResp(this->device_handle, *TEST_SLIM_HEADER_MESSAGES, 2, MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, *key_message, 1, MESSAGE_LENGTH);

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

    SendBufferToDevice(this->device_handle, *msg2, 1, MESSAGE_LENGTH);

    //SendBufferToDevice(this->device_handle, TEST_SLIM_MESSAGES, 1, MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, *END_BULK_UPDATE_MESSAGES, END_BULK_UPDATE_MESSAGE_COUNT, MESSAGE_LENGTH);
}


}
