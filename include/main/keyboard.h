#pragma once
#include <string>
#include <unordered_map>
#include <windows.h>
#include <memory>
#include "usb_functions.h"
#include "keyboards/abstract_keyboard.h"
#include "misc.h"


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

     // convenience
     void TurnOnActiveKeys();
     void TurnOffActiveKeys();

     /////////////////////
     // The Driver API! //
     /////////////////////

     // Set's the active keys for the keyboard.  When functions like SetKeysOnOff are invoked, only active keys set by this function are effected
     void SetActiveKeys(const std::vector<std::string>& key_names);
     // Recommended only for use in testing.  TODO: make private or something
     void SetActiveKeyIds(char* key_ids, UINT8 n_keys);
     // TODO: make private since the driver should be calculating the key_id
     void SetActiveKeyId(int index, char key_id);
     char GetActiveKeyId(int index);

     // Set's the active keys to their on state
     void SetKeysOnOff(KeyValue key_value);
     void SetKeysOnOff(KeyValue key_value, unsigned char* messages_sent);
     

     // utility
     void Dispose();
     bool Found();

     // Below is not well defined yet and may be removed...
     void BlinkActiveKeys(int n, int interval);
     void TurnOnActiveKeys(unsigned char* messages_sent);
     void SetKeysRGB(unsigned char r, unsigned char g, unsigned char b);
     void SetKeyRGB(char key_id, unsigned char r, unsigned char g, unsigned char b);
     void Blink(int n, int interval);
     
private:
     HANDLE device_handle;
     short vid; //  = 0x05ac
     short pid; //  = 0x024f
     char active_key_ids[256] = { 0 };
     UINT8 n_active_keys = 0;
     KeyboardModel keyboard_model;

     // This class is injected and provides functionality specific to the given keyboard model
     AbstractKeyboard* keyboard_spec;

     bool AccessDeviceHandle();

     void SetupKeyboardModel(KeyboardModel keyboard_model);
};
