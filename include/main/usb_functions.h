#pragma once
#include <vector>
#include <windows.h>
#include "misc.h"

namespace blink {


void PrintMessagesInBuffer(const unsigned char* buffer, size_t message_count, size_t message_length);

// Private...
void PrintMessageInBuffer(const unsigned char* buffer, size_t i, size_t message_length);

/**
 * Searches for a USB device with a matching vid, pid combination. 
 *
 * @param vid The Vendor ID of the device to filter for
 * @param pid The Product ID of the device to filter for
 * @param target_device_path A c-string representing the device path that should be interfaced with
 * @return HANDLE
 */
HANDLE SearchForDevice(short vid, short pid, const char* target_device_path);

/**
 * Sends a buffer to a USB device.  This function will only issue a SetReport message for each packet sent.
 *
 * @param deviceHandle The handle to the device to send the messages to
 * @param messages_ptr The message data to send
 * @param messageCount The number of messages to send as a packet
 * @param messageLength The size in bytes of each packet
 * @return HANDLE
 */
void SendBufferToDevice(
    HANDLE deviceHandle, const unsigned char* messages_ptr,
    size_t messageCount, size_t messageLength
);

/**
 * Sends a buffer to a USB device.  This function will issue both a SetReport and GetReport for each packet sent.
 *
 * @param deviceHandle The handle to the device to send the messages to
 * @param messages_ptr The message data to send
 * @param messageCount The number of messages to send as a packet
 * @param messageLength The size in bytes of each packet
 * @return HANDLE
 */
void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, const unsigned char* messages,
    size_t messageCount, size_t messageLength
);

/**
 * Lists available USB devices currently attached to the machine.  Available devices are listed in 
 * the known_keyboards array.  
 *
 * @return std::vector<KeyboardInfo> A list of available, currently connected keyboards
 */
std::vector<KeyboardInfo> ListAvailableKeyboards();
    

}
