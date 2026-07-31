#pragma once
#include "usb_io.h"

#include <vector>
#include <string>
#include <cstring>

namespace keylt {

struct OpEntry {
    // NOTE: don't name this member `interface` — the Windows SDK defines it as a
    // macro (`#define interface struct` in combaseapi.h), which breaks MSVC builds.
    std::string iface;      // "control" or "data"
    std::string operation;  // "send_feature", "get_feature", "write", "read_ack", "sleep"
    size_t size;
    int sleep_ms = 0;       // >0 for sleep entries
};

class RecordingUsbIO : public IUsbIO {
public:
    // When true, a synthetic "sleep 1ms" entry is recorded after every
    // SendFeatureReport — matching the hardware-required 1ms inter-packet delay.
    bool record_sleeps = false;

    bool Open(short vid, short pid, const char* control_interface, const char* data_interface) override {
        return true;
    }

    int SendFeatureReport(const unsigned char* data, size_t length) override {
        bin_data_.insert(bin_data_.end(), data, data + length);
        ops_.push_back({"control", "send_feature", length, 0});
        if (record_sleeps) {
            ops_.push_back({"control", "sleep", 0, 1});
        }
        return 0;
    }

    int GetFeatureReport(unsigned char* buffer, size_t length) override {
        memset(buffer, 0, length);
        ops_.push_back({"control", "get_feature", length, 0});
        return (int)length;
    }

    int WriteData(const unsigned char* data, size_t length) override {
        bin_data_.insert(bin_data_.end(), data, data + length);
        ops_.push_back({"data", "write", length, 0});
        return (int)length;
    }

    int ReadData(unsigned char* buffer, size_t length, int timeout_ms) override {
        memset(buffer, 0, length);
        ops_.push_back({"data", "read_ack", length, 0});
        return (int)length;
    }

    void Close() override {}

    const std::vector<uint8_t>& GetBinaryData() const { return bin_data_; }
    const std::vector<OpEntry>& GetOps() const { return ops_; }

private:
    std::vector<uint8_t> bin_data_;
    std::vector<OpEntry> ops_;
};

} // namespace keylt
