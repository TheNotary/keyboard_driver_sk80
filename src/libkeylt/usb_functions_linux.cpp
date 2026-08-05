#include "usb_functions.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cwchar>
#include <unistd.h>

#include <hidapi/hidapi.h>

#include "messages.h"
#include "keyboard.h"
#include "keyboards/known_keyboards.h"

namespace keylt {

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

void CloseDeviceHandle(DeviceHandle deviceHandle) {
    if (deviceHandle)
        hid_close(static_cast<hid_device*>(deviceHandle));
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

// Renders hidapi's wide error string on the narrow stream. Mixing std::cerr and
// std::wcerr on the same underlying FILE has undefined orientation, so convert.
static void ReportHidFailure(const char* what, int result, hid_device* dev) {
    std::cerr << what << " failed, result: " << result;

    const wchar_t* detail = hid_error(dev);
    if (detail) {
        char narrow[256];
        const size_t written = std::wcstombs(narrow, detail, sizeof(narrow) - 1);
        if (written != static_cast<size_t>(-1)) {
            narrow[written] = '\0';
            std::cerr << " (" << narrow << ")";
        }
    }
    std::cerr << std::endl;
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
        // Reported rather than swallowed: a silent failure here is
        // indistinguishable from a successful LED update, because every caller
        // up to the C ABI returns void/OK regardless.
        ReportHidFailure("hid_send_feature_report", result, dev);
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
    if (result < 0) {
        // The response itself is discarded, but a failure is still evidence
        // that the control interface is not answering.
        ReportHidFailure("hid_get_feature_report", result, dev);
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

DeviceHandle SearchForLcdDataDevice(short vid, short pid, int control_interface_number) {
    struct hid_device_info *devs, *cur_dev;
    hid_device* device = nullptr;

    devs = hid_enumerate(vid, pid);
    cur_dev = devs;

    while (cur_dev) {
        if (cur_dev->interface_number != control_interface_number) {
            device = hid_open_path(cur_dev->path);
            if (device) {
                break;
            }
        }
        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);
    return static_cast<DeviceHandle>(device);
}

int SendFeatureReport(DeviceHandle deviceHandle, const unsigned char* data, size_t length) {
    return SendPayloadBytesToDevice(deviceHandle, data, length);
}

int GetFeatureReport(DeviceHandle deviceHandle, unsigned char* buffer, size_t length) {
    if (!deviceHandle) {
        std::cerr << "GetFeatureReport: null device handle" << std::endl;
        return -1;
    }
    hid_device* dev = static_cast<hid_device*>(deviceHandle);
    return hid_get_feature_report(dev, buffer, length);
}

int WriteDataToDevice(DeviceHandle deviceHandle, const unsigned char* data, size_t length) {
    if (!deviceHandle) {
        std::cerr << "WriteDataToDevice: null device handle" << std::endl;
        return -1;
    }
    hid_device* dev = static_cast<hid_device*>(deviceHandle);
    return hid_write(dev, data, length);
}

int ReadFromDevice(DeviceHandle deviceHandle, unsigned char* buffer, size_t length, int timeout_ms) {
    if (!deviceHandle) {
        std::cerr << "ReadFromDevice: null device handle" << std::endl;
        return -1;
    }
    hid_device* dev = static_cast<hid_device*>(deviceHandle);
    return hid_read_timeout(dev, buffer, length, timeout_ms);
}

} // namespace keylt
