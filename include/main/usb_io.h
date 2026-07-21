#pragma once
#include <cstddef>
#include <cstdint>
#include "platform.h"

namespace blink {

class IUsbIO {
public:
    virtual ~IUsbIO() = default;

    virtual bool Open(short vid, short pid, const char* control_interface) = 0;
    virtual int SendFeatureReport(const unsigned char* data, size_t length) = 0;
    virtual int GetFeatureReport(unsigned char* buffer, size_t length) = 0;
    virtual int WriteData(const unsigned char* data, size_t length) = 0;
    virtual int ReadData(unsigned char* buffer, size_t length, int timeout_ms) = 0;
    virtual void Close() = 0;
};

class RealUsbIO : public IUsbIO {
public:
    ~RealUsbIO() override { Close(); }

    bool Open(short vid, short pid, const char* control_interface) override;
    int SendFeatureReport(const unsigned char* data, size_t length) override;
    int GetFeatureReport(unsigned char* buffer, size_t length) override;
    int WriteData(const unsigned char* data, size_t length) override;
    int ReadData(unsigned char* buffer, size_t length, int timeout_ms) override;
    void Close() override;

private:
    DeviceHandle control_handle_ = nullptr;
    DeviceHandle data_handle_ = nullptr;
};

} // namespace blink
