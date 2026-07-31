#include "usb_io.h"
#include "usb_functions.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

namespace keylt {

bool RealUsbIO::Open(short vid, short pid, const char* control_interface, const char* data_interface) {
    control_handle_ = SearchForDevice(vid, pid, control_interface);
    if (!control_handle_) {
        std::cerr << "Failed to open control interface" << std::endl;
        return false;
    }

    data_handle_ = SearchForDevice(vid, pid, data_interface);
    if (!data_handle_) {
        std::cerr << "Failed to open LCD data interface" << std::endl;
        return false;
    }

    return true;
}

int RealUsbIO::SendFeatureReport(const unsigned char* data, size_t length) {
    return keylt::SendFeatureReport(control_handle_, data, length);
}

int RealUsbIO::GetFeatureReport(unsigned char* buffer, size_t length) {
    return keylt::GetFeatureReport(control_handle_, buffer, length);
}

int RealUsbIO::WriteData(const unsigned char* data, size_t length) {
    return keylt::WriteDataToDevice(data_handle_, data, length);
}

int RealUsbIO::ReadData(unsigned char* buffer, size_t length, int timeout_ms) {
    return keylt::ReadFromDevice(data_handle_, buffer, length, timeout_ms);
}

void RealUsbIO::Close() {
    if (control_handle_) {
        keylt::CloseDeviceHandle(control_handle_);
        control_handle_ = nullptr;
    }
    if (data_handle_) {
        keylt::CloseDeviceHandle(data_handle_);
        data_handle_ = nullptr;
    }
}

} // namespace keylt
