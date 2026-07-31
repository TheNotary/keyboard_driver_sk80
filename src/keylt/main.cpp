#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "keylt.h"

// Exit codes
static constexpr int EXIT_USAGE = 1;
static constexpr int EXIT_DEVICE = 2;
static constexpr int EXIT_IO = 3;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void print_usage() {
    fprintf(stderr,
        "Usage: keylt <command> [options]\n"
        "\n"
        "Commands:\n"
        "  on                Turn all keys on (full brightness)\n"
        "  off               Turn all keys off\n"
        "  set_on_lights     Turn specified keys on, all others off\n"
        "  lcd <path>        Upload image or GIF to keyboard LCD\n"
        "  list              List connected keyboards\n"
        "\n"
        "Options:\n"
        "  --keyboard VID:PID   Target a specific keyboard (see 'keylt list')\n"
        "  --help               Show this help message\n"
    );
}

struct ParsedArgs {
    std::string command;
    std::string keyboard_selector; // "VID:PID" or empty
    std::string positional;        // e.g. path for lcd
    std::vector<uint8_t> key_ids;  // for set_on_lights
};

static bool parse_vid_pid(const std::string& s, uint16_t* vid, uint16_t* pid) {
    unsigned v, p;
    if (sscanf(s.c_str(), "%x:%x", &v, &p) != 2) return false;
    if (v > 0xFFFF || p > 0xFFFF) return false;
    *vid = static_cast<uint16_t>(v);
    *pid = static_cast<uint16_t>(p);
    return true;
}

static bool parse_key_ids(const char* str, std::vector<uint8_t>& out) {
    out.clear();
    const char* p = str;
    while (*p) {
        char* end;
        long val = strtol(p, &end, 10);
        if (end == p || val < 0 || val > 255) return false;
        out.push_back(static_cast<uint8_t>(val));
        p = end;
        if (*p == ',') p++;
        else if (*p != '\0') return false;
    }
    return !out.empty();
}

static ParsedArgs parse_args(int argc, char* argv[]) {
    ParsedArgs args;
    if (argc < 2) return args;

    int i = 1;
    // First non-flag argument is the command
    if (argv[i][0] != '-') {
        args.command = argv[i];
        i++;
    }

    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_usage();
            exit(0);
        } else if (arg == "--keyboard" && i + 1 < argc) {
            args.keyboard_selector = argv[++i];
        } else if (arg == "--key-ids" && i + 1 < argc) {
            if (!parse_key_ids(argv[++i], args.key_ids)) {
                fprintf(stderr, "Error: Invalid --key-ids value. Expected comma-separated integers (e.g. 1,2,3,4)\n");
                exit(EXIT_USAGE);
            }
        } else if (arg[0] != '-' && args.positional.empty()) {
            args.positional = arg;
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", arg.c_str());
            print_usage();
            exit(EXIT_USAGE);
        }
        i++;
    }
    return args;
}

// Resolves which keyboard to target. Returns the model enum value.
// Exits on error (no device, ambiguous, or selector mismatch).
static keylt_model resolve_keyboard(const std::string& selector) {
    // First pass: count available keyboards
    size_t count = 0;
    keylt_status st = keylt_list_available_keyboards(nullptr, 0, &count);
    if (st != KEYLT_OK && st != KEYLT_ERR_BUFFER_TOO_SMALL) {
        fprintf(stderr, "Error: Failed to enumerate keyboards: %s\n", keylt_last_error_message());
        exit(EXIT_DEVICE);
    }

    if (count == 0) {
        fprintf(stderr, "Error: No keyboards found.\n"
                        "Make sure a supported keyboard is connected and you have permissions (try sudo).\n");
        exit(EXIT_DEVICE);
    }

    std::vector<keylt_keyboard_info> infos(count);
    st = keylt_list_available_keyboards(infos.data(), count, &count);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to enumerate keyboards: %s\n", keylt_last_error_message());
        exit(EXIT_DEVICE);
    }

    if (!selector.empty()) {
        uint16_t vid, pid;
        if (!parse_vid_pid(selector, &vid, &pid)) {
            fprintf(stderr, "Error: Invalid --keyboard value '%s'. Expected format: VID:PID (e.g. 05ac:024f)\n",
                    selector.c_str());
            exit(EXIT_USAGE);
        }
        for (size_t i = 0; i < count; i++) {
            if (infos[i].vid == vid && infos[i].pid == pid) {
                return static_cast<keylt_model>(infos[i].model);
            }
        }
        fprintf(stderr, "Error: No keyboard matching %04x:%04x found.\nConnected keyboards:\n", vid, pid);
        for (size_t i = 0; i < count; i++) {
            fprintf(stderr, "  %s  %04x:%04x\n", infos[i].display_name, infos[i].vid, infos[i].pid);
        }
        exit(EXIT_DEVICE);
    }

    // No selector provided
    if (count > 1) {
        fprintf(stderr, "Error: Multiple keyboards found. Use --keyboard VID:PID to select one.\n"
                        "Connected keyboards:\n");
        for (size_t i = 0; i < count; i++) {
            fprintf(stderr, "  %s  %04x:%04x\n", infos[i].display_name, infos[i].vid, infos[i].pid);
        }
        exit(EXIT_USAGE);
    }

    return static_cast<keylt_model>(infos[0].model);
}

// Collects all key IDs for a model into a vector.
static std::vector<uint8_t> all_key_ids(keylt_model model) {
    size_t count = 0;
    keylt_model_key_count(model, &count);
    std::vector<uint8_t> ids(count);
    for (size_t i = 0; i < count; i++) {
        keylt_model_key_at(model, i, nullptr, &ids[i]);
    }
    return ids;
}

// ---------------------------------------------------------------------------
// Subcommands
// ---------------------------------------------------------------------------

static int cmd_list() {
    size_t count = 0;
    keylt_status st = keylt_list_available_keyboards(nullptr, 0, &count);
    if (st != KEYLT_OK && st != KEYLT_ERR_BUFFER_TOO_SMALL) {
        fprintf(stderr, "Error: Failed to enumerate keyboards: %s\n", keylt_last_error_message());
        return EXIT_DEVICE;
    }

    if (count == 0) {
        fprintf(stderr, "No keyboards found.\n");
        return 0;
    }

    std::vector<keylt_keyboard_info> infos(count);
    st = keylt_list_available_keyboards(infos.data(), count, &count);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to enumerate keyboards: %s\n", keylt_last_error_message());
        return EXIT_DEVICE;
    }

    for (size_t i = 0; i < count; i++) {
        printf("%s\t%04x:%04x\n", infos[i].display_name, infos[i].vid, infos[i].pid);
    }
    return 0;
}

static int cmd_on(const std::string& selector) {
    keylt_model model = resolve_keyboard(selector);
    keylt_keyboard* kb = nullptr;

    keylt_status st = keylt_keyboard_open(model, &kb);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to open keyboard: %s\n", keylt_last_error_message());
        return EXIT_DEVICE;
    }

    st = keylt_keyboard_connect(kb);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to connect to keyboard: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_DEVICE;
    }

    auto ids = all_key_ids(model);
    st = keylt_keyboard_set_active_key_ids(kb, ids.data(), ids.size());
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to select keys: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_IO;
    }

    st = keylt_keyboard_set_keys(kb, KEYLT_KEY_ON);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to set keys: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_IO;
    }

    keylt_keyboard_close(kb);
    return 0;
}

static int cmd_off(const std::string& selector) {
    keylt_model model = resolve_keyboard(selector);
    keylt_keyboard* kb = nullptr;

    keylt_status st = keylt_keyboard_open(model, &kb);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to open keyboard: %s\n", keylt_last_error_message());
        return EXIT_DEVICE;
    }

    st = keylt_keyboard_connect(kb);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to connect to keyboard: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_DEVICE;
    }

    auto ids = all_key_ids(model);
    st = keylt_keyboard_set_active_key_ids(kb, ids.data(), ids.size());
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to select keys: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_IO;
    }

    st = keylt_keyboard_set_keys(kb, KEYLT_KEY_OFF);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to set keys: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_IO;
    }

    keylt_keyboard_close(kb);
    return 0;
}

static int cmd_set_on_lights(const std::string& selector, const std::vector<uint8_t>& key_ids) {
    if (key_ids.empty()) {
        fprintf(stderr, "Error: --key-ids is required for set_on_lights.\n"
                        "Usage: keylt set_on_lights --key-ids 1,2,3,4\n");
        return EXIT_USAGE;
    }

    keylt_model model = resolve_keyboard(selector);
    keylt_keyboard* kb = nullptr;

    keylt_status st = keylt_keyboard_open(model, &kb);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to open keyboard: %s\n", keylt_last_error_message());
        return EXIT_DEVICE;
    }

    st = keylt_keyboard_connect(kb);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to connect to keyboard: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_DEVICE;
    }

    st = keylt_keyboard_set_active_key_ids(kb, key_ids.data(), key_ids.size());
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Invalid key IDs: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_USAGE;
    }

    st = keylt_keyboard_set_keys(kb, KEYLT_KEY_ON);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to set keys: %s\n", keylt_last_error_message());
        keylt_keyboard_close(kb);
        return EXIT_IO;
    }

    keylt_keyboard_close(kb);
    return 0;
}

static int cmd_lcd(const std::string& selector, const std::string& path) {
    if (path.empty()) {
        fprintf(stderr, "Error: Path to image or GIF is required.\n"
                        "Usage: keylt lcd <path>\n");
        return EXIT_USAGE;
    }

    // Verify file exists
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "Error: File not found: %s\n", path.c_str());
        return EXIT_USAGE;
    }
    fclose(f);

    // LCD is SK80-specific; resolve keyboard to validate model
    keylt_model model = resolve_keyboard(selector);
    if (model != KEYLT_MODEL_SK80) {
        fprintf(stderr, "Error: LCD upload is only supported on the SK80 keyboard.\n");
        return EXIT_DEVICE;
    }

    uint16_t vid, pid;
    const char* control_iface;
    const char* data_iface;
    keylt_sk80_lcd_defaults(&vid, &pid, &control_iface, &data_iface);

    keylt_lcd* lcd = nullptr;
    keylt_status st = keylt_lcd_open(nullptr, vid, pid, control_iface, data_iface, &lcd);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to open LCD: %s\n", keylt_last_error_message());
        return EXIT_DEVICE;
    }

    st = keylt_lcd_connect(lcd);
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Failed to connect to LCD: %s\n", keylt_last_error_message());
        keylt_lcd_close(lcd);
        return EXIT_DEVICE;
    }

    st = keylt_lcd_upload_image(lcd, path.c_str());
    if (st != KEYLT_OK) {
        fprintf(stderr, "Error: Upload failed: %s\n", keylt_last_error_message());
        keylt_lcd_close(lcd);
        return EXIT_IO;
    }

    keylt_lcd_close(lcd);
    return 0;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    ParsedArgs args = parse_args(argc, argv);

    if (args.command.empty() || args.command == "help") {
        print_usage();
        return args.command == "help" ? 0 : EXIT_USAGE;
    }

    if (args.command == "list") {
        return cmd_list();
    } else if (args.command == "on") {
        return cmd_on(args.keyboard_selector);
    } else if (args.command == "off") {
        return cmd_off(args.keyboard_selector);
    } else if (args.command == "set_on_lights") {
        return cmd_set_on_lights(args.keyboard_selector, args.key_ids);
    } else if (args.command == "lcd") {
        return cmd_lcd(args.keyboard_selector, args.positional);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", args.command.c_str());
        print_usage();
        return EXIT_USAGE;
    }
}
