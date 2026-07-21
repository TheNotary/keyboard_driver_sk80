#pragma once
#include "usb_io.h"

#include <vector>
#include <string>
#include <cstring>

namespace blink {

struct OpEntry {
    std::string interface;  // "control" or "data"
    std::string operation;  // "send_feature", "get_feature", "write", "read_ack"
    size_t size;
};

class RecordingUsbIO : public IUsbIO {
public:
    bool Open(short vid, short pid, const char* control_interface) override {
        return true;
    }

    int SendFeatureReport(const unsigned char* data, size_t length) override {
        bin_data_.insert(bin_data_.end(), data, data + length);
        ops_.push_back({"control", "send_feature", length});
        return 0;
    }

    int GetFeatureReport(unsigned char* buffer, size_t length) override {
        memset(buffer, 0, length);
        ops_.push_back({"control", "get_feature", length});
        return (int)length;
    }

    int WriteData(const unsigned char* data, size_t length) override {
        bin_data_.insert(bin_data_.end(), data, data + length);
        ops_.push_back({"data", "write", length});
        return (int)length;
    }

    int ReadData(unsigned char* buffer, size_t length, int timeout_ms) override {
        memset(buffer, 0, length);
        ops_.push_back({"data", "read_ack", length});
        return (int)length;
    }

    void Close() override {}

    const std::vector<uint8_t>& GetBinaryData() const { return bin_data_; }
    const std::vector<OpEntry>& GetOps() const { return ops_; }

private:
    std::vector<uint8_t> bin_data_;
    std::vector<OpEntry> ops_;
};

} // namespace blink
