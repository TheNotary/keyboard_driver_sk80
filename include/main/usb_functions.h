#pragma once
#include <vector>
#include "platform.h"
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
DeviceHandle SearchForDevice(short vid, short pid, const char* target_device_path);

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
    DeviceHandle deviceHandle, const unsigned char* messages_ptr,
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
    DeviceHandle deviceHandle, const unsigned char* messages,
    size_t messageCount, size_t messageLength
);

/**
 * Lists available USB devices currently attached to the machine.  Available devices are listed in 
 * the known_keyboards array.  
 *
 * @return std::vector<KeyboardInfo> A list of available, currently connected keyboards
 */
std::vector<KeyboardInfo> ListAvailableKeyboards();

/**
 * Searches for an LCD data HID interface by enumerating all devices matching vid/pid
 * and skipping the control interface.
 *
 * @param vid The Vendor ID of the device
 * @param pid The Product ID of the device
 * @param control_interface_number The interface number to skip (the control interface)
 * @return DeviceHandle to the LCD data interface or nullptr
 */
DeviceHandle SearchForLcdDataDevice(short vid, short pid, int control_interface_number);

/**
 * Sends a feature report to the device (public wrapper).
 *
 * @param deviceHandle The handle to the device
 * @param data The feature report data (first byte is report ID)
 * @param length The length of the data
 * @return 0 on success, non-zero on failure
 */
int SendFeatureReport(DeviceHandle deviceHandle, const unsigned char* data, size_t length);

/**
 * Reads a feature report from the device.
 *
 * @param deviceHandle The handle to the device
 * @param buffer Buffer to fill with the response (first byte is report ID)
 * @param length The size of the buffer
 * @return Number of bytes read, or -1 on failure
 */
int GetFeatureReport(DeviceHandle deviceHandle, unsigned char* buffer, size_t length);

/**
 * Writes raw data to a device via HID output report.
 *
 * @param deviceHandle The handle to the device
 * @param data The data to write (first byte is report ID)
 * @param length The length of the data
 * @return Number of bytes written, or -1 on failure
 */
int WriteDataToDevice(DeviceHandle deviceHandle, const unsigned char* data, size_t length);

/**
 * Reads data from a device via HID input report with a timeout.
 *
 * @param deviceHandle The handle to the device
 * @param buffer Buffer to fill with the received data
 * @param length The size of the buffer
 * @param timeout_ms Timeout in milliseconds (-1 for blocking)
 * @return Number of bytes read, 0 on timeout, or -1 on failure
 */
int ReadFromDevice(DeviceHandle deviceHandle, unsigned char* buffer, size_t length, int timeout_ms);
    

}
