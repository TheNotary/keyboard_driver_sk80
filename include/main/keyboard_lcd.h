#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "platform.h"
#include "usb_io.h"

namespace blink {

class KeyboardLcd {
public:
    KeyboardLcd(IUsbIO* io, short vid, short pid, const char* control_interface, const char* data_interface);
    ~KeyboardLcd() { Dispose(); }

    bool ConnectToDevice();
    bool UploadImage(const std::string& image_path);
    void Dispose();
    bool Found();

private:
    IUsbIO* io_;
    bool connected_ = false;
    short vid, pid;
    const char* control_interface;
    const char* data_interface;

    static constexpr int W = 160;
    static constexpr int H = 96;
    static constexpr int PAGE_SIZE = 4096;
    static constexpr int MAX_FRAMES = 255;

    bool SendStart();
    bool SendImageConfig(uint8_t n_frames, uint16_t n_pages);
    bool SendDataPages(const uint8_t* data, size_t total_size);
    bool SendSave();

    std::vector<uint8_t> LoadAndConvertImage(const std::string& path, int& n_frames);
    static void RGB888ToRGB565(const uint8_t* rgb, uint8_t* out, int width, int height);
};

} // namespace blink
