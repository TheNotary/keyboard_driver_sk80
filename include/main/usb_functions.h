#pragma once
#include <windows.h>

void PrintWideString(const char* buffer, int bufferLen);

HANDLE SearchForDevice(short vid, short pid);

void SendBufferToDevice(
    HANDLE deviceHandle, unsigned char* messages,
    size_t messageCount, size_t messageLength
);

void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, unsigned char messages[][65],
    size_t messageCount, size_t messageLength
);
