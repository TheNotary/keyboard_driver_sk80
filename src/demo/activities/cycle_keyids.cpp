#include "cycle_keyids.h"

#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
static inline void Sleep(int ms) { usleep(ms * 1000); }
#endif

#include "keylt_loader.h"

using std::cout;
using std::endl;


namespace demo {


enum class InputAction { None, Prev, Next, PrintBuffer, Quit };

#ifdef _WIN32
static InputAction ReadInputAction() {
    if (GetAsyncKeyState(VK_DOWN) & 0x8000
        || GetAsyncKeyState(VK_LEFT) & 0x8000)
        return InputAction::Prev;

    if (GetAsyncKeyState(VK_UP) & 0x8000
        || GetAsyncKeyState(VK_RIGHT) & 0x8000)
        return InputAction::Next;

    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        return InputAction::PrintBuffer;

    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        return InputAction::Quit;

    return InputAction::None;
}
#else
static InputAction ReadInputAction() {
    unsigned char ch;
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    if (n != 1) return InputAction::None;

    if (ch == 27) { // ESC — could be a bare ESC or start of an arrow key sequence
        unsigned char seq[2];
        // Arrow keys send ESC [ X atomically; bytes are already in the buffer
        if (read(STDIN_FILENO, &seq[0], 1) == 1 && seq[0] == '[') {
            if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                switch (seq[1]) {
                    case 'A': return InputAction::Next; // Up
                    case 'B': return InputAction::Prev; // Down
                    case 'C': return InputAction::Next; // Right
                    case 'D': return InputAction::Prev; // Left
                }
            }
        }
        return InputAction::Quit; // Bare ESC
    }

    switch (ch) {
        case 'a': case 'h': case ',': return InputAction::Prev;
        case 'd': case 'l': case '.': return InputAction::Next;
        case ' ':                      return InputAction::PrintBuffer;
        case 'q':                      return InputAction::Quit;
        default:                       return InputAction::None;
    }
}
#endif


void PrintKeyId(char key_id) {
    cout << "\r" << std::string(12, ' ');
    cout << "\r" << "KeyId: " << std::dec << (int)key_id;
}

char IncrementKeyId(char value, int incrementation, UINT8 max_key_id) {
    // if we don't cast result to unsigned, it screws up the '>' comparisons,
    // making 0xff -1 instead of 255!
    // We don't need to return unsigned char since the calling code does implicit cast
    // If we pass in value = 0xff, isn't that going to be a different computation
    // whether we're signed or unsigned???

    unsigned char result = ((unsigned)value) + incrementation;

    if (result > max_key_id)  // If we're incrementing passed max_index
        return 1;             // then overflow the char to 1 after incrementing max_index

    if (result == 0)        // It is only possible to reach 0 if we're incrementing by -1
        return max_key_id;  // Therefore overflow to max_index

    return result;
}

int CycleKeyIds(keylt::KeyboardInfo keyboard) {
    cout << "CycleKeyIds Debug Mode:" << endl
        << "The key id will be shown on the screen, and the LED for that key will be switched "
        << "on making mapping the keyboard easy.  "
        << endl << endl
        << "Press left/right (or a/d) to cycle through the keyId to test.  " << endl
        << "Press Space to print the buffer that was last transmitted to the keyboard" << endl
        << "Press ESC (or q) to exit"
        << endl << endl;

#ifndef _WIN32
    // Set terminal to raw mode for non-blocking input
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
#endif

    std::vector<unsigned char> messages_sent(keyboard.BULK_LED_VALUE_MESSAGES_COUNT * keyboard.MESSAGE_LENGTH);
    char key_ids[] = { 0x01 };

    PrintKeyId(key_ids[0]);
    CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent.data(), keyboard);

    while (true) {
        InputAction action = ReadInputAction();

        switch (action) {
            case InputAction::Prev:
                key_ids[0] = IncrementKeyId(key_ids[0], -1, keyboard.max_key_id);
                CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent.data(), keyboard);
                PrintKeyId(key_ids[0]);
                Sleep(40);
                break;

            case InputAction::Next:
                key_ids[0] = IncrementKeyId(key_ids[0], 1, keyboard.max_key_id);
                CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent.data(), keyboard);
                PrintKeyId(key_ids[0]);
                Sleep(40);
                break;

            case InputAction::PrintBuffer:
                CallPrintMessagesInBuffer(messages_sent.data(), keyboard.BULK_LED_VALUE_MESSAGES_COUNT, keyboard.MESSAGE_LENGTH);
                Sleep(200);
                break;

            case InputAction::Quit:
#ifndef _WIN32
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
                return 0;

            case InputAction::None:
                break;
        }

        Sleep(10);
    }
}


}
