#pragma once
#include <windows.h>

void PrintWideString(const char* buffer, int bufferLen);

HANDLE SearchForDevice(char* vid, char* pid);

void SendBufferToDevice(
    HANDLE deviceHandle, unsigned char messages[][65],
    size_t messageCount, size_t messageLength
);

void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, unsigned char messages[][65],
    size_t messageCount, size_t messageLength
);
