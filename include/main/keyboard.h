#pragma once
#include "usb_functions.h"
#include <string>
#include <windows.h>

/**
 * @brief The Keyboard class allows you to interface with the LEDs on the keyboard.
 *
 * This class allows you to change the RGB values of the keys on the keyboard by 
 * sending bulk data to the keyboard.
 */
class Keyboard {
public:
    // Name GetNameObject(const std::string& name) const; // interesting syntax, right?
    Keyboard();
    bool AccessDeviceHandle();
    HANDLE GetDeviceHandle();
    void SetKeyRGB(char keyId, unsigned char r, unsigned char g, unsigned char b);
    void Blink(int n, int interval);
    void Dispose();
    bool Found();
private:
    HANDLE device_handle;
    short vid = 0x05ac;
    short pid = 0x024f;
};

