#include "usb_functions.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <hidsdi.h>

#include "messages.h"
#include "keyboard.h"
#include "keyboards/known_keyboards.h"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

namespace blink {


void PrintMessageInBuffer(const unsigned char* buffer, size_t i, size_t message_length) {
    printf("\n");
    // Print the wacky first byte out of order because it helps expose the true nature of our messages
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

HANDLE open_device(LPCSTR device_path) {
    LPCSTR lpFileName = device_path;
    DWORD dwDesiredAccess = 0xc0000000;
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
    DWORD dwCreationDisposition = OPEN_EXISTING;
    DWORD dwFlagsAndAttributes = 0x40000000;
    HANDLE hTemplateFile = NULL;

    return CreateFile(lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

void PrintDeviceDetails(HANDLE hDev,
    SP_DEVICE_INTERFACE_DETAIL_DATA* deviceDetails,
    SP_DEVICE_INTERFACE_DATA deviceInfo,
    SP_DEVINFO_DATA device_info_data,
    HIDD_ATTRIBUTES deviceAttributes
) {
    // Save std::cout settings so we can mutate them as needed in this function
    std::ios_base::fmtflags f(std::cout.flags()); 

    char productString[256];
    ZeroMemory(productString, sizeof(productString));

    if (!HidD_GetProductString(hDev, productString, sizeof(productString))) {
        std::cerr << "Failed calling HidD_GetProductString\n";
        return;
    }

    char mfcString[256];
    ZeroMemory(mfcString, sizeof(mfcString));
    if (!HidD_GetManufacturerString(hDev, mfcString, sizeof(mfcString))) {
        std::cerr << "Failed calling HidD_GetManufacturerString\n";
        return;
    }

    std::cout
        << "PRINTING DETAILS FOR " << deviceDetails->DevicePath << std::endl;

    std::wcout
        << "productString: " << (wchar_t*)productString << std::endl

        // This will say SONiX in unicode for our device
        << "mfcString: " << (wchar_t*)mfcString << std::endl;

    std::cout << std::hex << std::setfill('0')

            << "ID: "
                << std::setw(4) << deviceAttributes.VendorID
                << ":" 
                << std::setw(4) << deviceAttributes.ProductID << std::endl

            << "DevInst: 0x" 
                << std::setw(4) << device_info_data.DevInst << std::endl
        
            << "Reserved: 0x" 
                << std::setw(8) << device_info_data.Reserved << std::endl;

    GUID guid = device_info_data.ClassGuid;

    printf("ClassGuid: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n\n",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    // Reset the std::cout flags (so hexadicimal for instance isn't the representation for ints in subsequent cout invocations)
    std::cout.flags(f);
}

void DoAdditionalUsbThings(HANDLE hDev) {
    PHIDP_PREPARSED_DATA preparsedData;
    bool result = HidD_GetPreparsedData(hDev, &preparsedData);

    HIDP_CAPS capabilities;
    NTSTATUS capsResult = HidP_GetCaps(preparsedData, &capabilities);
    if (capsResult == 0x0011000000)
        printf("compared...");

    HidD_FreePreparsedData(preparsedData);
}

/*
* This functions can be passed a lambda allowing really clean re-use of the C++ logic!
* Return false from the caller's lambda to halt the enumeration and clean up the USB 
* stuff without closing the last handle.
* 
* Return true from the lambda to close the handle and continue enumerating until all devices have been enumerated.
**/ 
template <typename DeviceHandler>
void EnumerateDevices(DeviceHandler handleDevice) {
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);

    HDEVINFO deviceInfoList = SetupDiGetClassDevs(&hidGuid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if (deviceInfoList == INVALID_HANDLE_VALUE)
        return;

    const size_t DEVICE_DETAILS_SIZE = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + MAX_PATH;
    DWORD size = DEVICE_DETAILS_SIZE;
    SP_DEVICE_INTERFACE_DETAIL_DATA* deviceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA*)alloca(DEVICE_DETAILS_SIZE);
    deviceDetails->cbSize = sizeof(*deviceDetails);

    SP_DEVICE_INTERFACE_DATA deviceInfo;
    deviceInfo.cbSize = sizeof(deviceInfo);

    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(device_info_data);

    HIDD_ATTRIBUTES deviceAttributes;
    deviceAttributes.Size = sizeof(deviceAttributes);

    for (int i = 0; ; ++i) {
        HANDLE hDev = INVALID_HANDLE_VALUE;

        if (!SetupDiEnumDeviceInterfaces(deviceInfoList, 0, &hidGuid, i, &deviceInfo)) {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;
            else
                continue;
        }

        if (!SetupDiGetDeviceInterfaceDetail(deviceInfoList, &deviceInfo,
            deviceDetails, size, &size, &device_info_data)) {
            continue;
        }

        hDev = open_device(deviceDetails->DevicePath);
        if (hDev == INVALID_HANDLE_VALUE) {
            continue;
        }

        if (!HidD_GetAttributes(hDev, &deviceAttributes)) {
            CloseHandle(hDev);
            continue;
        }

        // PrintDeviceDetails(hDev, deviceDetails, deviceInfo, device_info_data, deviceAttributes);

        bool yield_result = handleDevice(hDev, deviceDetails, deviceAttributes, deviceInfo, device_info_data);

        if (!yield_result) {
            break;
        }
        CloseHandle(hDev);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoList);
}

HANDLE SearchForDevice(short vid, short pid, const char* target_device_path) {
    HANDLE resultHandle = 0;

    EnumerateDevices([&](HANDLE hDev, SP_DEVICE_INTERFACE_DETAIL_DATA* deviceDetails,
        HIDD_ATTRIBUTES& deviceAttributes,
        SP_DEVICE_INTERFACE_DATA& deviceInfo, SP_DEVINFO_DATA& device_info_data) -> bool {
            if (deviceAttributes.VendorID == vid && deviceAttributes.ProductID == pid) {
                if (strstr(deviceDetails->DevicePath, target_device_path)) {
                    resultHandle = hDev;
                    return false;
                }
            }
            return true;
        }
    );

    return resultHandle;
}

/*
 * Returns a list of keyboards that are attached to the system and are known to the program per known_keyboards.h
 */
std::vector<KeyboardInfo> ListAvailableKeyboards() {
    std::vector<KeyboardInfo> available_keyboards;

    EnumerateDevices([&](HANDLE hDev, SP_DEVICE_INTERFACE_DETAIL_DATA* deviceDetails,
        HIDD_ATTRIBUTES& deviceAttributes,
        SP_DEVICE_INTERFACE_DATA& deviceInfo, SP_DEVINFO_DATA& device_info_data) -> bool {
            for (size_t i = 0; i < known_keyboards.size(); i++) {
                KeyboardInfo known_keyboard = known_keyboards[i];
                if (deviceAttributes.VendorID == known_keyboard.vid && deviceAttributes.ProductID == known_keyboard.pid) {
                    // If we haven't already added this keyboard to available_keyboards, do so
                     if (std::find(available_keyboards.begin(), available_keyboards.end(), known_keyboard) == available_keyboards.end()) {
                         available_keyboards.push_back(known_keyboard);
                         return true;
                     }
                }
            }
            return true;
        }
    );

    return available_keyboards;
}

static int SendPayloadBytesToDevice(HANDLE deviceHandle, const UCHAR* payload, size_t payloadLength)
{
    // Set the feature report
    if (!HidD_SetFeature(deviceHandle, (PVOID)payload, payloadLength))
    {
        std::cerr << "Failed on SendPayloadBytesToDevice: " << GetLastError() << std::endl;
        CloseHandle(deviceHandle);
        return 1;
    }
    Sleep(1); // A sleep statement is crucial here, was set to 35ms, but 1ms seems enough
    return 0;
}

static int SwallowDeviceGetReport(HANDLE deviceHandle)
{
    UCHAR inputBuffer[65];
    ZeroMemory(inputBuffer, sizeof(inputBuffer));

    UCHAR outputBuffer[65] = { 0 };
    DWORD bytesReturned = 0x0000;
    OVERLAPPED overlapped = { 0 };

    DWORD        dwIoControlCode = 0x0b0192;
    LPVOID       lpInBuffer = &inputBuffer;
    DWORD        nInBufferSize = sizeof(inputBuffer);
    LPVOID       lpOutBuffer = &outputBuffer;
    DWORD        nOutBufferSize = sizeof(outputBuffer);
    LPDWORD      lpBytesReturned = &bytesReturned;

    if (!DeviceIoControl(deviceHandle, dwIoControlCode, lpInBuffer,
        nInBufferSize, lpOutBuffer, nOutBufferSize,
        lpBytesReturned, &overlapped))
    {
        std::cerr << "Failed to DeviceIoControl: " << GetLastError() << std::endl;
        CloseHandle(deviceHandle);
        return 1;
    }

    DWORD numberOfBytesTransferred;
    BOOL bWait = true;
    BOOL wasOverlapped = GetOverlappedResult(deviceHandle, &overlapped, &numberOfBytesTransferred, bWait);

    return 0;
}

static int SendPayloadBytesToDeviceAndGetResp(HANDLE deviceHandle, const UCHAR* message, size_t messageLength)
{
    SendPayloadBytesToDevice(deviceHandle, message, messageLength);
    SwallowDeviceGetReport(deviceHandle);
    return 0;
}

void SendBufferToDevice(
    HANDLE deviceHandle, const unsigned char* messages_ptr,
    size_t messageCount, size_t messageLength
) {
    const unsigned char (*messages)[65] = reinterpret_cast<const unsigned char (*)[65]>(messages_ptr);

    for (size_t i = 0; i < messageCount; i++)
    {
        //std::cout << "send_buffer_to_device " << i << std::endl;
        SendPayloadBytesToDevice(deviceHandle, messages[i], messageLength);
    }
}

void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, const unsigned char* messages_ptr,
    size_t messageCount, size_t messageLength
) {
    const unsigned char (*messages)[65] = reinterpret_cast<const unsigned char (*)[65]>(messages_ptr);
    for (size_t i = 0; i < messageCount; i++)
    {
        //std::cout << "send_buffer_to_device " << i << std::endl;
        SendPayloadBytesToDeviceAndGetResp(deviceHandle, messages[i], messageLength);
    }
}


}
