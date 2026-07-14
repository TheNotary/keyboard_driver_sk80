#include "usb_functions.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <hidapi/hidapi.h>

#include "messages.h"
#include "keyboard.h"
#include "keyboards/known_keyboards.h"

namespace blink {

void PrintMessageInBuffer(const unsigned char* buffer, size_t i, size_t message_length) {
    printf("\n");
    printf("0x%02x", buffer[i * message_length + 0]);
    for (size_t j = 1; j < message_length; j++) {
        if ((j-1) % 8 == 0)
            printf("\n");
        printf("0x%02x ", buffer[i * message_length + j]);
    }
}

void PrintMessagesInBuffer(
    const unsigned char* buffer,
    size_t message_count,
    size_t message_length
) {
    for (size_t i = 0; i < message_count; i++) {
        PrintMessageInBuffer(buffer, i, message_length);
        printf("\n");
    }
}

DeviceHandle SearchForDevice(short vid, short pid, const char* target_device_path) {
    struct hid_device_info *devs, *cur_dev;
    hid_device* device = nullptr;

    devs = hid_enumerate(vid, pid);
    cur_dev = devs;

    while (cur_dev) {
        // On Linux, target_device_path contains the target interface number as a string
        if (target_device_path && strlen(target_device_path) > 0) {
            int target_iface = atoi(target_device_path);
            if (cur_dev->interface_number == target_iface) {
                device = hid_open_path(cur_dev->path);
                break;
            }
        } else {
            // No specific interface filter — open first match
            device = hid_open_path(cur_dev->path);
            break;
        }
        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);
    return static_cast<DeviceHandle>(device);
}

std::vector<KeyboardInfo> ListAvailableKeyboards() {
    std::vector<KeyboardInfo> available_keyboards;

    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(0, 0);
    cur_dev = devs;

    while (cur_dev) {
        for (size_t i = 0; i < known_keyboards.size(); i++) {
            KeyboardInfo known_keyboard = known_keyboards[i];
            if (cur_dev->vendor_id == known_keyboard.vid &&
                cur_dev->product_id == known_keyboard.pid) {
                // If we haven't already added this keyboard, do so
                if (std::find(available_keyboards.begin(), available_keyboards.end(), known_keyboard) == available_keyboards.end()) {
                    available_keyboards.push_back(known_keyboard);
                }
            }
        }
        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);
    return available_keyboards;
}

// Send a single feature report to the device
static int SendPayloadBytesToDevice(DeviceHandle deviceHandle, const unsigned char* payload, size_t payloadLength)
{
    if (!deviceHandle) {
        std::cerr << "SendPayloadBytesToDevice: null device handle" << std::endl;
        return 1;
    }
    hid_device* dev = static_cast<hid_device*>(deviceHandle);
    
    int result = hid_send_feature_report(dev, payload, payloadLength);
    if (result < 0) {
        // Known hidapi-libusb quirk: may report failure even though data was sent.
        // LED changes typically still succeed on Linux. Not a critical error.
        return 1;
    }
    usleep(1000); // 1ms delay — critical for device stability (matches Windows Sleep(1))
    return 0;
}

// Read and discard a feature report response from the device.
static int SwallowDeviceGetReport(DeviceHandle deviceHandle)
{
    if (!deviceHandle) {
        std::cerr << "SwallowDeviceGetReport: null device handle" << std::endl;
        return 1;
    }
    hid_device* dev = static_cast<hid_device*>(deviceHandle);

    unsigned char buffer[65];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0x00; // Report ID

    int result = hid_get_feature_report(dev, buffer, sizeof(buffer));
    // std::cout << "Attempted hid_get_feature_report, result: " << result << std::endl;
    if (result < 0) {
        // Response is discarded anyway; failure here doesn't affect LED changes.
        return 1;
    }
    return 0;
}

static int SendPayloadBytesToDeviceAndGetResp(DeviceHandle deviceHandle, const unsigned char* message, size_t messageLength)
{
    SendPayloadBytesToDevice(deviceHandle, message, messageLength);
    SwallowDeviceGetReport(deviceHandle);
    return 0;
}

void SendBufferToDevice(
    DeviceHandle deviceHandle, const unsigned char* messages_ptr,
    size_t messageCount, size_t messageLength
) {
    const unsigned char (*messages)[65] = reinterpret_cast<const unsigned char (*)[65]>(messages_ptr);

    for (size_t i = 0; i < messageCount; i++)
    {
        SendPayloadBytesToDevice(deviceHandle, messages[i], messageLength);
    }
}

void SendBufferToDeviceAndGetResp(
    DeviceHandle deviceHandle, const unsigned char* messages_ptr,
    size_t messageCount, size_t messageLength
) {
    const unsigned char (*messages)[65] = reinterpret_cast<const unsigned char (*)[65]>(messages_ptr);
    for (size_t i = 0; i < messageCount; i++)
    {
        SendPayloadBytesToDeviceAndGetResp(deviceHandle, messages[i], messageLength);
    }
}

} // namespace blink
