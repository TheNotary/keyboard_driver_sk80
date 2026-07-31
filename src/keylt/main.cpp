#include <iostream>
#include <string>
#include <fstream>

#include "keyboard_lcd.h"
#include "usb_io.h"
#include "keyboards/sk80/constants_sk80.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: keylt lcd <image_or_gif_path>" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    // Verify file exists
    std::ifstream file(path);
    if (!file.good()) {
        std::cerr << "Error: File not found: " << path << std::endl;
        return 1;
    }
    file.close();

    keylt::RealUsbIO usb_io;
    keylt::KeyboardLcd lcd(&usb_io, sk80::VID, sk80::PID, sk80::target_device_path, sk80::lcd_data_device_path);

    std::cout << "Opening devices..." << std::endl;
    if (!lcd.ConnectToDevice()) {
        std::cerr << "Error: Could not connect to SK80 keyboard." << std::endl;
        std::cerr << "Make sure the keyboard is connected and you have permissions (try sudo)." << std::endl;
        return 1;
    }

    if (!lcd.UploadImage(path)) {
        std::cerr << "Error: Upload failed." << std::endl;
        return 1;
    }

    return 0;
}
