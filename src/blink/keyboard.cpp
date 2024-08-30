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


Keyboard::Keyboard()
{
    printf("Constructing\n");
    this->AccessDeviceHandle();
}

bool Keyboard::AccessDeviceHandle() {
    this->device_handle = SearchForDevice("05AC", "024F");
    //this->device_handle = SearchForDevice("0C45", "8006");
    return true;
}

HANDLE Keyboard::GetDeviceHandle() {
    return this->device_handle;
}

void Keyboard::SetKeyRGB(char keyId, unsigned char r, unsigned char g, unsigned char b) {
    std::cout << "Setting LED" << std::endl;

    TEST_SLIM_HEADER_MESSAGES[1][9] = 0x01; // number of packets to send in "bulk transfer"
    SendBufferToDeviceAndGetResp(this->device_handle, TEST_SLIM_HEADER_MESSAGES, 2, 65);


    TEST_SLIM_MESSAGES[0][54] = r;
    TEST_SLIM_MESSAGES[0][55] = g;
    TEST_SLIM_MESSAGES[0][56] = b;

    SendBufferToDevice(this->device_handle, TEST_SLIM_MESSAGES, 1, TURN_F12_RED_MESSAGE_LENGTH);

    SendBufferToDevice(this->device_handle, END_BULK_UPDATE_MESSAGES, END_BULK_UPDATE_MESSAGE_COUNT, END_BULK_UPDATE_MESSAGE_LENGTH);
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
