// SK80 LED protocol conformance test.
// Compares the current (keylt) SK80 implementation against golden fixtures
// generated from the original (blink) version to ensure no protocol drift.

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

#include "keyboard.h"
#include "recording_usb_io.h"
#include "keyboards/sk80/sk80.h"
#include "keyboards/sk80/constants_sk80.h"

#ifndef TEST_SOURCE_DIR
#error "TEST_SOURCE_DIR must be defined to locate reference files"
#endif

static std::string FixturePath(const std::string& name) {
    return std::string(TEST_SOURCE_DIR) + "/test/fixtures/sk80/" + name;
}

// Parse .times file: returns list of OpEntry (skipping comments).
static std::vector<keylt::OpEntry> ParseTimesFile(const std::string& path) {
    std::vector<keylt::OpEntry> ops;
    std::ifstream f(path);
    EXPECT_TRUE(f.is_open()) << "Could not open: " << path;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("sleep ", 0) == 0) {
            // Parse "sleep 1ms"
            int ms = 0;
            if (sscanf(line.c_str(), "sleep %dms", &ms) == 1) {
                ops.push_back({"control", "sleep", 0, ms});
            }
            continue;
        }

        // Format: <interface> <operation> <label> <size_bytes>
        std::istringstream ss(line);
        std::string iface, op, label;
        size_t size;
        if (ss >> iface >> op >> label >> size) {
            ops.push_back({iface, op, size, 0});
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

// ---------------------------------------------------------------------------
// Test infrastructure
// ---------------------------------------------------------------------------

struct SK80ConformanceScenario {
    std::string fixture_name;
    std::vector<char> key_ids;
    keylt::KeyValue key_value;
};

class SK80ConformanceTest : public ::testing::TestWithParam<SK80ConformanceScenario> {
protected:
    keylt::RecordingUsbIO recorder;

    void SetUp() override {
        recorder.record_sleeps = true;
    }

    void RunScenario(const SK80ConformanceScenario& scenario) {
        keylt::Keyboard keyboard(keylt::kSK80);
        keyboard.keyboard_spec->SetUsbIO(&recorder);
        keyboard.SetActiveKeyIds(scenario.key_ids.data(), static_cast<UINT8>(scenario.key_ids.size()));
        keyboard.SetKeysOnOff(scenario.key_value);
    }
};

// ---------------------------------------------------------------------------
// Operation sequence conformance
// ---------------------------------------------------------------------------

TEST_P(SK80ConformanceTest, OperationSequenceMatchesOriginal) {
    const auto& scenario = GetParam();
    RunScenario(scenario);

    auto ref_ops = ParseTimesFile(FixturePath(scenario.fixture_name + ".times"));
    const auto& cpp_ops = recorder.GetOps();

    ASSERT_EQ(cpp_ops.size(), ref_ops.size())
        << "Operation count mismatch for " << scenario.fixture_name
        << ": current=" << cpp_ops.size() << " original=" << ref_ops.size();

    for (size_t i = 0; i < ref_ops.size(); i++) {
        EXPECT_EQ(cpp_ops[i].iface, ref_ops[i].iface)
            << "Op " << i << " (" << scenario.fixture_name << "): interface mismatch";
        EXPECT_EQ(cpp_ops[i].operation, ref_ops[i].operation)
            << "Op " << i << " (" << scenario.fixture_name << "): operation mismatch";
        if (ref_ops[i].operation == "sleep") {
            EXPECT_EQ(cpp_ops[i].sleep_ms, ref_ops[i].sleep_ms)
                << "Op " << i << " (" << scenario.fixture_name << "): sleep duration mismatch";
        } else {
            EXPECT_EQ(cpp_ops[i].size, ref_ops[i].size)
                << "Op " << i << " (" << scenario.fixture_name << "): size mismatch";
        }
    }
}

// ---------------------------------------------------------------------------
// Binary payload conformance
// ---------------------------------------------------------------------------

TEST_P(SK80ConformanceTest, BinaryPayloadMatchesOriginal) {
    const auto& scenario = GetParam();
    RunScenario(scenario);

    auto ref_bin = LoadBinaryFile(FixturePath(scenario.fixture_name + ".bin"));
    const auto& cpp_bin = recorder.GetBinaryData();

    ASSERT_EQ(cpp_bin.size(), ref_bin.size())
        << "Total binary size mismatch for " << scenario.fixture_name
        << ": current=" << cpp_bin.size() << " original=" << ref_bin.size();

    for (size_t i = 0; i < ref_bin.size(); i++) {
        EXPECT_EQ(cpp_bin[i], ref_bin[i])
            << scenario.fixture_name << " byte " << i << " mismatch"
            << " (packet " << (i / 65) << ", offset " << (i % 65) << ")";
    }
}

// ---------------------------------------------------------------------------
// Test scenarios
// ---------------------------------------------------------------------------

INSTANTIATE_TEST_SUITE_P(
    SK80Protocol,
    SK80ConformanceTest,
    ::testing::Values(
        SK80ConformanceScenario{"sk80_single_key_on", {1}, keylt::kOn},
        SK80ConformanceScenario{"sk80_single_key_off", {1}, keylt::kOff},
        SK80ConformanceScenario{"sk80_multi_key_on", {1, 2, 37}, keylt::kOn},
        SK80ConformanceScenario{"sk80_max_keyid_on", {121}, keylt::kOn},
        SK80ConformanceScenario{"sk80_all_fkeys_on", {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, keylt::kOn}
    ),
    [](const ::testing::TestParamInfo<SK80ConformanceScenario>& info) {
        return info.param.fixture_name;
    }
);
