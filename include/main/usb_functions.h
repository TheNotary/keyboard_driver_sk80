#pragma once
#include <vector>
#include <windows.h>
#include "misc.h"


void PrintWideString(const char* buffer, int bufferLen);
void PrintMessagesInBuffer(const unsigned char* buffer, size_t message_count, size_t message_length);

// Private...
void PrintMessageInBuffer(const unsigned char* buffer, size_t i, size_t message_length);

HANDLE SearchForDevice(short vid, short pid, const char* target_device_path);

void SendBufferToDevice(
    HANDLE deviceHandle, const unsigned char* messages_ptr,
    size_t messageCount, size_t messageLength
);

void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, const unsigned char* messages,
    size_t messageCount, size_t messageLength
);

std::vector<KeyboardInfo> ListAvailableKeyboards();
