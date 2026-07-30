#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include "keyboard_lcd.h"

#include <iostream>
#include <fstream>
#include <cctype>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <thread>

namespace keylt {

KeyboardLcd::KeyboardLcd(IUsbIO* io, short vid, short pid, const char* control_interface, const char* data_interface)
    : io_(io), vid(vid), pid(pid), control_interface(control_interface), data_interface(data_interface) {}

bool KeyboardLcd::ConnectToDevice() {
    if (!io_->Open(vid, pid, control_interface, data_interface)) {
        return false;
    }
    connected_ = true;
    return true;
}

void KeyboardLcd::Dispose() {
    if (connected_) {
        io_->Close();
        connected_ = false;
    }
}

bool KeyboardLcd::Found() {
    return connected_;
}

bool KeyboardLcd::SendStart() {
    unsigned char cmd[65] = {};
    cmd[0] = 0x00; // Report ID
    cmd[1] = 0x04;
    cmd[2] = 0x18;

    int result = io_->SendFeatureReport(cmd, sizeof(cmd));
    if (result != 0) {
        std::cerr << "Failed to send START command" << std::endl;
        return false;
    }

    unsigned char resp[65] = {};
    resp[0] = 0x00;
    int bytes_read = io_->GetFeatureReport(resp, sizeof(resp));
    if (bytes_read > 0) {
        printf("  Handshake: ");
        for (int i = 1; i <= 8 && i < bytes_read; i++)
            printf("%02X ", resp[i]);
        printf("\n");
    }
    return true;
}

bool KeyboardLcd::SendImageConfig(uint8_t n_frames, uint16_t n_pages) {
    unsigned char cmd[65] = {};
    cmd[0] = 0x00; // Report ID
    cmd[1] = 0x04;
    cmd[2] = 0x72;
    cmd[3] = n_frames;
    cmd[9] = n_pages & 0xFF;
    cmd[10] = (n_pages >> 8) & 0xFF;

    int result = io_->SendFeatureReport(cmd, sizeof(cmd));
    if (result != 0) {
        std::cerr << "Failed to send IMAGE_CFG command" << std::endl;
        return false;
    }

    unsigned char resp[65] = {};
    resp[0] = 0x00;
    int bytes_read = io_->GetFeatureReport(resp, sizeof(resp));
    if (bytes_read > 0) {
        printf("  Handshake: ");
        for (int i = 1; i <= 8 && i < bytes_read; i++)
            printf("%02X ", resp[i]);
        printf("\n");
    }
    return true;
}

bool KeyboardLcd::SendDataPages(const uint8_t* data, size_t total_size) {
    size_t n_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t i = 0; i < n_pages; i++) {
        uint8_t page_buf[1 + PAGE_SIZE]; // Report ID + page data
        page_buf[0] = 0x00; // Report ID

        size_t offset = i * PAGE_SIZE;
        size_t remaining = total_size - offset;
        size_t copy_len = std::min(remaining, (size_t)PAGE_SIZE);

        memcpy(page_buf + 1, data + offset, copy_len);
        if (copy_len < PAGE_SIZE) {
            memset(page_buf + 1 + copy_len, 0xFF, PAGE_SIZE - copy_len);
        }

        int result = io_->WriteData(page_buf, sizeof(page_buf));
        if (result < 0) {
            std::cerr << "Failed to write data page " << i << std::endl;
            return false;
        }

        // Pacing between pages is required for device stability.
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        unsigned char ack[64] = {};
        io_->ReadData(ack, sizeof(ack), 300);

        if ((i + 1) % 20 == 0 || i == n_pages - 1) {
            printf("  %zu/%zu ACK=%02X %02X %02X %02X\n",
                   i + 1, n_pages, ack[0], ack[1], ack[2], ack[3]);
        }
    }
    return true;
}

bool KeyboardLcd::SendSave() {
    unsigned char cmd[65] = {};
    cmd[0] = 0x00; // Report ID
    cmd[1] = 0x04;
    cmd[2] = 0x02;

    int result = io_->SendFeatureReport(cmd, sizeof(cmd));
    if (result != 0) {
        std::cerr << "Failed to send SAVE command" << std::endl;
        return false;
    }

    unsigned char resp[65] = {};
    resp[0] = 0x00;
    int bytes_read = io_->GetFeatureReport(resp, sizeof(resp));
    if (bytes_read > 0) {
        printf("  Handshake: ");
        for (int i = 1; i <= 8 && i < bytes_read; i++)
            printf("%02X ", resp[i]);
        printf("\n");
    }
    return true;
}

void KeyboardLcd::RGB888ToRGB565(const uint8_t* rgb, uint8_t* out, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        uint8_t r = rgb[i * 3 + 0];
        uint8_t g = rgb[i * 3 + 1];
        uint8_t b = rgb[i * 3 + 2];
        uint16_t v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        out[i * 2 + 0] = v & 0xFF;        // Low byte (LE)
        out[i * 2 + 1] = (v >> 8) & 0xFF; // High byte (LE)
    }
}

std::vector<uint8_t> KeyboardLcd::LoadAndConvertImage(const std::string& path, int& n_frames) {
    // Check if it's a GIF by extension
    bool is_gif = false;
    {
        std::string lower_path = path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        is_gif = (lower_path.size() >= 4 && lower_path.substr(lower_path.size() - 4) == ".gif");
    }

    std::vector<uint8_t> result;
    // 256-byte header
    std::vector<uint8_t> header(256, 0);

    if (is_gif) {
        // Load entire file into memory for stbi_load_gif_from_memory
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            n_frames = 0;
            return {};
        }
        size_t file_size = static_cast<size_t>(file.tellg());
        file.seekg(0);
        std::vector<uint8_t> file_data(file_size);
        file.read(reinterpret_cast<char*>(file_data.data()), file_size);

        int* delays = nullptr;
        int x, y, z, comp;
        uint8_t* frames_data = stbi_load_gif_from_memory(
            file_data.data(), (int)file_size,
            &delays, &x, &y, &z, &comp, 3);

        if (!frames_data) {
            std::cerr << "Failed to load GIF: " << stbi_failure_reason() << std::endl;
            n_frames = 0;
            return {};
        }

        n_frames = std::min(z, MAX_FRAMES);
        if (z > MAX_FRAMES) {
            std::cerr << "Warning: GIF has " << z << " frames, capping at " << MAX_FRAMES << std::endl;
        }

        header[0] = (uint8_t)n_frames;

        // Pixel data for all frames
        std::vector<uint8_t> all_pixels;
        all_pixels.reserve(n_frames * W * H * 2);

        for (int i = 0; i < n_frames; i++) {
            int ms = (delays && delays[i] > 0) ? delays[i] : 100;
            // Store delay as ms/2, clamped to at least 1
            header[1 + i] = (uint8_t)std::max(1, ms / 2);

            // Resize frame to LCD dimensions
            uint8_t* frame_src = frames_data + (size_t)i * x * y * 3;
            std::vector<uint8_t> resized(W * H * 3);
            stbir_resize_uint8(
                frame_src, x, y, 0,
                resized.data(), W, H, 0,
                3);

            // Convert to RGB565
            std::vector<uint8_t> rgb565(W * H * 2);
            RGB888ToRGB565(resized.data(), rgb565.data(), W, H);

            all_pixels.insert(all_pixels.end(), rgb565.begin(), rgb565.end());
        }

        stbi_image_free(frames_data);
        free(delays);

        result.reserve(header.size() + all_pixels.size());
        result.insert(result.end(), header.begin(), header.end());
        result.insert(result.end(), all_pixels.begin(), all_pixels.end());
    } else {
        // Static image
        int x, y, comp;
        uint8_t* img_data = stbi_load(path.c_str(), &x, &y, &comp, 3);
        if (!img_data) {
            std::cerr << "Failed to load image: " << stbi_failure_reason() << std::endl;
            n_frames = 0;
            return {};
        }

        n_frames = 1;
        header[0] = 1;
        header[1] = 100; // Default delay

        // Resize to LCD dimensions
        std::vector<uint8_t> resized(W * H * 3);
        stbir_resize_uint8(
            img_data, x, y, 0,
            resized.data(), W, H, 0,
            3);

        stbi_image_free(img_data);

        // Convert to RGB565
        std::vector<uint8_t> rgb565(W * H * 2);
        RGB888ToRGB565(resized.data(), rgb565.data(), W, H);

        result.reserve(header.size() + rgb565.size());
        result.insert(result.end(), header.begin(), header.end());
        result.insert(result.end(), rgb565.begin(), rgb565.end());
    }

    // Pad to PAGE_SIZE boundary
    size_t padded_size = ((result.size() + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    result.resize(padded_size, 0xFF);

    return result;
}

bool KeyboardLcd::UploadImage(const std::string& image_path) {
    std::cout << std::string(50, '=') << std::endl;
    std::cout << "S-K80 Display Uploader" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    // Load and convert image
    int n_frames = 0;
    std::vector<uint8_t> image_data = LoadAndConvertImage(image_path, n_frames);
    if (image_data.empty() || n_frames == 0) {
        std::cerr << "Failed to load image" << std::endl;
        return false;
    }

    bool is_gif = n_frames > 1;
    uint16_t n_pages = (uint16_t)(image_data.size() / PAGE_SIZE);

    std::cout << "\n" << (is_gif ? "GIF" : "Image") << ": " << n_frames << " frame(s)" << std::endl;
    std::cout << "Pages: " << n_pages << " (" << image_data.size() << " bytes)" << std::endl;

    // Step 1: START
    std::cout << "\n[1/4] START..." << std::endl;
    if (!SendStart()) return false;

    // Step 2: IMAGE_CFG
    std::cout << "[2/4] IMAGE_CFG..." << std::endl;
    if (!SendImageConfig((uint8_t)n_frames, n_pages)) return false;

    // Step 3: DATA pages
    std::cout << "[3/4] DATA (" << n_pages << " pages)..." << std::endl;
    if (!SendDataPages(image_data.data(), image_data.size())) return false;

    // Wait 3 seconds before saving
    std::cout << "  Waiting 3s..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Step 4: SAVE
    std::cout << "[4/4] SAVE..." << std::endl;
    if (!SendSave()) return false;

    std::cout << "\nDone! " << n_frames << " frame(s) uploaded!" << std::endl;
    std::cout << "Check your display!" << std::endl;
    return true;
}

} // namespace keylt
