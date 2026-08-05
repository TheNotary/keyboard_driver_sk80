#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

#include "keyboard_lcd.h"
#include "recording_usb_io.h"
#include "keyboards/sk80/constants_sk80.h"

#ifndef TEST_SOURCE_DIR
#error "TEST_SOURCE_DIR must be defined to locate reference files"
#endif

static std::string SourcePath(const std::string& relative) {
    return std::string(TEST_SOURCE_DIR) + "/" + relative;
}

// Parse my-output.times: skip comment (#) and sleep lines.
// Returns list of {interface, operation, size}.
static std::vector<keylt::OpEntry> ParseTimesFile(const std::string& path) {
    std::vector<keylt::OpEntry> ops;
    std::ifstream f(path);
    EXPECT_TRUE(f.is_open()) << "Could not open: " << path;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.rfind("sleep ", 0) == 0) continue;

        // Format: <interface> <operation> <label> <size_bytes>
        std::istringstream ss(line);
        std::string iface, op, label;
        size_t size;
        if (ss >> iface >> op >> label >> size) {
            ops.push_back({iface, op, size});
        }
    }
    return ops;
}

static std::vector<uint8_t> LoadBinaryFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    EXPECT_TRUE(f.is_open()) << "Could not open: " << path;
    size_t sz = static_cast<size_t>(f.tellg());
    f.seekg(0);
    std::vector<uint8_t> data(sz);
    f.read(reinterpret_cast<char*>(data.data()), sz);
    return data;
}

class GifToSK80Test : public ::testing::Test {
protected:
    keylt::RecordingUsbIO recorder;
    std::vector<uint8_t> ref_bin;
    std::vector<keylt::OpEntry> ref_ops;

    void SetUp() override {
        // Load reference files from Python dry-run
        ref_bin = LoadBinaryFile(SourcePath("test/fixtures/my-output.bin"));
        ref_ops = ParseTimesFile(SourcePath("test/fixtures/my-output.times"));

        // Run C++ upload with recording IO
        keylt::KeyboardLcd lcd(&recorder, sk80::VID, sk80::PID, sk80::target_device_path, sk80::lcd_data_device_path);
        lcd.ConnectToDevice();
        bool ok = lcd.UploadImage(SourcePath("samples/29.gif"));
        ASSERT_TRUE(ok) << "UploadImage failed";
    }
};

// Test 1: Operation sequence matches the Python dry-run
TEST_F(GifToSK80Test, OperationSequenceMatchesPythonDryRun) {
    const auto& cpp_ops = recorder.GetOps();

    ASSERT_EQ(cpp_ops.size(), ref_ops.size())
        << "Operation count mismatch: C++=" << cpp_ops.size()
        << " Python=" << ref_ops.size();

    for (size_t i = 0; i < ref_ops.size(); i++) {
        EXPECT_EQ(cpp_ops[i].iface, ref_ops[i].iface)
            << "Op " << i << ": interface mismatch";
        EXPECT_EQ(cpp_ops[i].operation, ref_ops[i].operation)
            << "Op " << i << ": operation mismatch";
        EXPECT_EQ(cpp_ops[i].size, ref_ops[i].size)
            << "Op " << i << ": size mismatch";
    }
}

// Test 2: Feature report command bytes match exactly
TEST_F(GifToSK80Test, CommandBytesMatchPythonDryRun) {
    const auto& cpp_bin = recorder.GetBinaryData();

    ASSERT_EQ(cpp_bin.size(), ref_bin.size())
        << "Total binary size mismatch: C++=" << cpp_bin.size()
        << " Python=" << ref_bin.size();

    // START command: first 65 bytes
    for (size_t i = 0; i < 65; i++) {
        EXPECT_EQ(cpp_bin[i], ref_bin[i])
            << "START command byte " << i << " mismatch";
    }

    // IMAGE_CFG command: bytes 65-129
    for (size_t i = 65; i < 130; i++) {
        EXPECT_EQ(cpp_bin[i], ref_bin[i])
            << "IMAGE_CFG command byte " << i << " mismatch";
    }

    // SAVE command: last 65 bytes
    size_t save_offset = ref_bin.size() - 65;
    for (size_t i = save_offset; i < ref_bin.size(); i++) {
        EXPECT_EQ(cpp_bin[i], ref_bin[i])
            << "SAVE command byte " << i << " mismatch";
    }
}

// Test 3: Image header matches exactly, page structure is correct
TEST_F(GifToSK80Test, PageHeaderAndStructureMatch) {
    const auto& cpp_bin = recorder.GetBinaryData();

    ASSERT_EQ(cpp_bin.size(), ref_bin.size())
        << "Total binary size mismatch — different page count";

    // Page data starts at offset 130 (after START + IMAGE_CFG feature reports)
    // Each page is 4097 bytes (1 byte report ID + 4096 data)
    // Image header is the first 256 bytes of page 0 data (offsets 131..386)
    constexpr size_t page_data_start = 130;
    constexpr size_t header_offset = page_data_start + 1; // skip report ID

    // Compare 256-byte image header exactly (frame count, delays)
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(cpp_bin[header_offset + i], ref_bin[header_offset + i])
            << "Image header byte " << i << " mismatch";
    }

    // Verify all page report IDs are 0x00
    size_t save_offset = ref_bin.size() - 65;
    size_t n_pages = (save_offset - page_data_start) / 4097;
    for (size_t p = 0; p < n_pages; p++) {
        size_t rid_offset = page_data_start + p * 4097;
        EXPECT_EQ(cpp_bin[rid_offset], 0x00)
            << "Page " << p << " report ID should be 0x00";
    }

    // Verify padding bytes at end of last page are 0xFF
    // (both implementations pad to PAGE_SIZE with 0xFF)
    size_t last_page_start = page_data_start + (n_pages - 1) * 4097 + 1;
    // Check last 64 bytes of last page are padding
    for (size_t i = last_page_start + 4096 - 64; i < last_page_start + 4096; i++) {
        EXPECT_EQ(cpp_bin[i], ref_bin[i])
            << "Padding byte at offset " << i << " mismatch";
    }
}

// Hardware test: validates real device discovery succeeds when keyboard is connected.
// Skipped unless KEYBOARD_ATTACHED=1 is set in environment.
TEST(GifToSK80HardwareTest, DeviceDiscoveryFindsControlAndDataInterfaces) {
    const char* env = std::getenv("KEYBOARD_ATTACHED");
    if (!env || std::string(env) != "1") {
        GTEST_SKIP() << "Skipped: set KEYBOARD_ATTACHED=1 to run hardware tests";
    }

    keylt::RealUsbIO usb_io;
    bool opened = usb_io.Open(sk80::VID, sk80::PID,
                              sk80::target_device_path,
                              sk80::lcd_data_device_path);
    ASSERT_TRUE(opened) << "Failed to open control (interface "
        << sk80::target_device_path << ") and data (interface "
        << sk80::lcd_data_device_path << ") devices";

    // Send START command and verify we get a valid handshake back
    unsigned char cmd[65] = {};
    cmd[0] = 0x00; // Report ID
    cmd[1] = 0x04;
    cmd[2] = 0x18;

    int send_result = usb_io.SendFeatureReport(cmd, sizeof(cmd));
    EXPECT_EQ(send_result, 0) << "SendFeatureReport(START) failed";

    unsigned char resp[65] = {};
    resp[0] = 0x00;
    int bytes_read = usb_io.GetFeatureReport(resp, sizeof(resp));
    EXPECT_GT(bytes_read, 0) << "GetFeatureReport returned no data";

    // The handshake echoes back the command bytes (04 18)
    EXPECT_EQ(resp[1], 0x04) << "Handshake byte 1 should be 0x04";
    EXPECT_EQ(resp[2], 0x18) << "Handshake byte 2 should be 0x18";

    usb_io.Close();
}
