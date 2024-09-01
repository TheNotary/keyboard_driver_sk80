#pragma once
#include "usb_functions.h"
#include <string>
#include <unordered_map>
#include <windows.h>

enum KeyboardModel { SK80, MK84, RK84 };
enum KeyValue { kOn, kOff };

struct DeviceInfo {
    short vid;
    short pid;
};

// Setup a mapping between, say "f12" and 0x0d (the ID for the key as known by the hardware)
using KeyNameKeyIdPair = std::unordered_map<std::string, char>;
extern std::unordered_map<KeyboardModel, KeyNameKeyIdPair> keyname_keyid_mappings;

using KeyValueBytesPair = std::unordered_map<KeyValue, char>;
extern std::unordered_map<KeyboardModel, KeyValueBytesPair> on_off_mappings;


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
     void SetActiveKeyIds(char* key_ids, UINT8 n_keys);
     void SetActiveKeys(const std::vector<std::string>& key_names);
     void BlinkActiveKeys(int n, int interval);
     void TurnOnActiveKeys();
     void SetKeysRGB(unsigned char r, unsigned char g, unsigned char b);
     void SetKeyRGB(char key_id, unsigned char r, unsigned char g, unsigned char b);
     void Blink(int n, int interval);
     void Dispose();
     bool Found();
     short GetPid();
private:
     HANDLE device_handle;
     short vid; //  = 0x05ac
     short pid; //  = 0x024f
     char active_key_ids[256] = { 0 };
     UINT8 n_active_keys = 0;
     KeyboardModel keyboard_model;

     struct DeviceInfo {
         short vid;
         short pid;
     };

     std::unordered_map<KeyboardModel, DeviceInfo> device_mappings = {
         { SK80, {0x05ac, 0x024f} },
         { MK84, {0x0000, 0x0000} },
         { RK84, {0x258a, 0x00c0} }
     };

     void SetupKeyboardModel(KeyboardModel keyboard_model);
};
