#include "../include/usb_functions.h"
#include <windows.h>
#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <setupapi.h>
#include <stdio.h>
#include <hidsdi.h>
#include "../include/messages.h"
#include "../include/keyboard.h"


static void PrintWideString(const char* buffer, int bufferLen) {
    for (int i = 0; i < bufferLen; i += 2) {
        if (buffer[i] == 0)
            break;
        printf("%c", buffer[i]);
    }
}

HANDLE SearchForDevice(char* vid, char* pid) {
    GUID hidGuid;
    HDEVINFO deviceInfoList;
    const size_t DEVICE_DETAILS_SIZE = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + MAX_PATH;
    SP_DEVICE_INTERFACE_DETAIL_DATA* deviceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA*)alloca(DEVICE_DETAILS_SIZE);
    deviceDetails->cbSize = sizeof(*deviceDetails);

    HidD_GetHidGuid(&hidGuid);
    deviceInfoList = SetupDiGetClassDevs(&hidGuid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if (deviceInfoList == INVALID_HANDLE_VALUE)
        return 0;

    for (int i = 0; ; ++i) {
        SP_DEVICE_INTERFACE_DATA deviceInfo;
        SP_DEVINFO_DATA device_info_data;
        DWORD size = DEVICE_DETAILS_SIZE;
        HIDD_ATTRIBUTES deviceAttributes;
        HANDLE hDev = INVALID_HANDLE_VALUE;

        deviceInfo.cbSize = sizeof(deviceInfo);
        device_info_data.cbSize = sizeof(device_info_data);

        if (!SetupDiEnumDeviceInterfaces(deviceInfoList, 0, &hidGuid, i,
            &deviceInfo))
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;
            else
                continue;

        if (!SetupDiGetDeviceInterfaceDetail(deviceInfoList, &deviceInfo,
            deviceDetails, size, &size, &device_info_data))
            continue;

        LPCSTR lpFileName = deviceDetails->DevicePath;
        DWORD dwDesiredAccess = 0xc0000000;
        DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
        DWORD dwCreationDisposition = OPEN_EXISTING;
        DWORD dwFlagsAndAttributes = 0x40000000;
        HANDLE hTemplateFile = NULL;

        hDev = CreateFile(lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
        if (hDev == INVALID_HANDLE_VALUE) {
            continue;
        }

        // TODO: I don't think I need to do this apparently... was just copying from reference app...
        PHIDP_PREPARSED_DATA preparsedData;
        bool result = HidD_GetPreparsedData(hDev, &preparsedData);

        HIDP_CAPS capabilities;
        NTSTATUS capsResult = HidP_GetCaps(preparsedData, &capabilities);
        if (capsResult == 0x0011000000)
            fmt::print("compared...");

        HidD_FreePreparsedData(preparsedData);

        deviceAttributes.Size = sizeof(deviceAttributes);
        if (!HidD_GetAttributes(hDev, &deviceAttributes)) {
            printf("Failed calling HidD_GetAttributes");
            continue;
        }

        char productString[256];
        ZeroMemory(productString, sizeof(productString));

        if (!HidD_GetProductString(hDev, productString, sizeof(productString))) {
            printf("Failed calling HidD_GetProductString");
            continue;
        }

        char mfcString[256];
        ZeroMemory(mfcString, sizeof(mfcString));
        if (!HidD_GetManufacturerString(hDev, mfcString, sizeof(mfcString))) {
            printf("Failed calling HidD_GetManufacturerString");
            continue;
        }

        char vendorId[16];
        char productId[16];
        char devicePath[1024];

        sprintf(vendorId, "%04X", (unsigned)deviceAttributes.VendorID);
        sprintf(productId, "%04X", (unsigned)deviceAttributes.ProductID);
        sprintf(devicePath, "%s", deviceDetails->DevicePath);

        printf("productString: ");
        PrintWideString(productString, sizeof(productString));
        printf("\n");

        // This will say SONiX in unicode for our device
        printf("mfcString: ");
        PrintWideString(mfcString, sizeof(mfcString));
        printf("\n");

        fprintf(stdout, "Opening device %s\n", devicePath);

        printf(vendorId);
        printf(":");
        printf(productId);
        printf("\n");

        if (strstr((char*)vendorId, vid) && strstr((char*)productId, pid)) {
            printf("DevInst: %04x\n", device_info_data.DevInst);
            printf("DevInst: %x\n", device_info_data.Reserved);

            GUID guid = device_info_data.ClassGuid;

            printf("ClassGuid: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

            printf("productString: ");
            PrintWideString(productString, sizeof(productString));
            printf("\n");

            // This will say SONiX in unicode for our device
            printf("mfcString: ");
            PrintWideString(mfcString, sizeof(mfcString));
            printf("\n");

            fprintf(stdout, "Opening device %s\n", devicePath);

            printf(vendorId);
            printf(":");
            printf(productId);
            printf("\n");

            //                     "\\\\?\\hid#vid_05ac&pid_024f&mi_00#8&16781069&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}\\kbd"
            //if (strstr(devicePath, "\\\\?\\hid#vid_05ac&pid_024f&mi_03#8&6cca243&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"))
            //{
            return hDev; // We apparently can return the first one, though it seems the one with mi_03 is the lucky one to latch onto?  or does it matter?
            //}
        }
        CloseHandle(hDev);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoList);
    return 0;
}

static int SendPayloadBytesToDevice(HANDLE deviceHandle, const UCHAR* payload, size_t payloadLength)
{
    // Set the feature report
    if (!HidD_SetFeature(deviceHandle, (PVOID)payload, payloadLength))
    {
        std::cerr << "Failed on send_payload_bytes_to_device: " << GetLastError() << std::endl;
        CloseHandle(deviceHandle);
        return 1;
    }
    Sleep(1); // A sleep statement is crucial here, was set to 35ms, but 1ms seems enough
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
    HANDLE deviceHandle, unsigned char messages[][65],
    size_t messageCount, size_t messageLength
) {
    for (size_t i = 0; i < messageCount; i++)
    {
        //std::cout << "send_buffer_to_device " << i << std::endl;
        SendPayloadBytesToDevice(deviceHandle, messages[i], messageLength);
    }
}

void SendBufferToDeviceAndGetResp(
    HANDLE deviceHandle, unsigned char messages[][65],
    size_t messageCount, size_t messageLength
) {
    for (size_t i = 0; i < messageCount; i++)
    {
        //std::cout << "send_buffer_to_device " << i << std::endl;
        SendPayloadBytesToDeviceAndGetResp(deviceHandle, messages[i], messageLength);
    }
}

