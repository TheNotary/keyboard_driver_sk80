#pragma once
#include "usb_functions.h"
#include <string>
#include <unordered_map>
#include <windows.h>
#include <memory>
#include <keyboards/keyboard_base.h>
#include <misc.h>


/**
 * @brief The Keyboard class allows you to interface with the LEDs on the keyboard.
 *
 * This class allows you to change the RGB values of the keys on the keyboard by
 * sending bulk data to the keyboard.
 */
class Keyboard {
public:
     // Name GetNameObject(const std::string& name) const; // interesting syntax, right?
     Keyboard(KeyboardModel keyboard_model);
     bool AccessDeviceHandle();
     HANDLE GetDeviceHandle();
     void SetKeysOnOff(KeyValue key_value);
     void SetKeysOnOff(KeyValue key_value, unsigned char messages_sent[3][65]);
     void SetActiveKeyIds(char* key_ids, UINT8 n_keys);
     void SetActiveKeys(const std::vector<std::string>& key_names);
     void BlinkActiveKeys(int n, int interval);
     void TurnOnActiveKeys(unsigned char messages_sent[3][65]);
     void TurnOnActiveKeys();
     void TurnOffActiveKeys();
     void SetKeysRGB(unsigned char r, unsigned char g, unsigned char b);
     void SetKeyRGB(char key_id, unsigned char r, unsigned char g, unsigned char b);
     void Blink(int n, int interval);
     void Dispose();
     bool Found();
     char GetActiveKeyId(int index);
     void SetActiveKeyId(int index, char key_id);
     short GetPid();
private:
     HANDLE device_handle;
     short vid; //  = 0x05ac
     short pid; //  = 0x024f
     char active_key_ids[256] = { 0 };
     UINT8 n_active_keys = 0;
     KeyboardModel keyboard_model;

     // This class is injected and provides functionality specific to the given keyboard model
     KeyboardBase* keyboard_spec;

     void SetupKeyboardModel(KeyboardModel keyboard_model);
     void SetBytesInPacket(unsigned char* messages, KeyValue key_value, char* active_key_ids, UINT8 n_active_keys) const;

};
