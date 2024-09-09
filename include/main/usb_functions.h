#pragma once
#include <windows.h>
#include <vector>
#include "misc.h"


void PrintWideString(const char* buffer, int bufferLen);
void PrintMessageInBuffer(unsigned char* buffer, size_t i, size_t message_length);
void PrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length);

HANDLE SearchForDevice(short vid, short pid, const char* target_device_path);

void SendBufferToDevice(
    HANDLE deviceHandle, unsigned char* messages,
    size_t messageCount, size_t messageLength
);

void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, unsigned char messages[][65],
    size_t messageCount, size_t messageLength
);

std::vector<KeyboardInfo> ListAvailableKeyboards();
