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

// Non-blocking key read for Linux terminal
static int kbhit_read() {
    unsigned char ch;
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    if (n == 1) return ch;
    return -1;
}
#endif

#include "blink_loader.h"

using std::cout;
using std::endl;


namespace demo {


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

int CycleKeyIds(blink::KeyboardInfo keyboard) {
    cout << "CycleKeyIds Debug Mode:" << endl
        << "The key id will be shown on the screen, and the LED for that key will be switched "
        << "on making mapping the keyboard easy.  "
        << endl << endl
#ifdef _WIN32
        << "Press left or right to cycle through the keyId to test.  " << endl
        << "Press Space to print the buffer that was last transmitted to the keyboard" << endl
        << "Press escape to exit"
#else
        << "Press a/d to cycle through the keyId to test.  " << endl
        << "Press Space to print the buffer that was last transmitted to the keyboard" << endl
        << "Press q or ESC to exit"
#endif
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

    //std::vector<std::string> key_names = { "f11" };

    unsigned char* messages_sent = new unsigned char [keyboard.BULK_LED_VALUE_MESSAGES_COUNT * keyboard.MESSAGE_LENGTH];

    //unsigned char messages_sent[3][keyboard.MESSAGE_LENGTH] = { 0 };
    char key_ids[] = { 0x01 };

    PrintKeyId(key_ids[0]);
    CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);

    while (true) {
#ifdef _WIN32
        if (GetAsyncKeyState(VK_DOWN) & 0x8000
            || GetAsyncKeyState(VK_LEFT) & 0x8000) { // PREV
            key_ids[0] = IncrementKeyId(key_ids[0], -1, keyboard.max_key_id);
            CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);
            PrintKeyId(key_ids[0]);

            Sleep(40); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000
            || GetAsyncKeyState(VK_RIGHT) & 0x8000) { // NEXT
            key_ids[0] = IncrementKeyId(key_ids[0], 1, keyboard.max_key_id);
            CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);
            PrintKeyId(key_ids[0]);

            Sleep(40);
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {  // print packet buffer
            CallPrintMessagesInBuffer(messages_sent, keyboard.BULK_LED_VALUE_MESSAGES_COUNT, keyboard.MESSAGE_LENGTH);

            Sleep(200);
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return 0;
        }

        Sleep(10);
#else
        int ch = kbhit_read();
        if (ch == 'a' || ch == 'h' || ch == ',') { // PREV (a/h/comma)
            key_ids[0] = IncrementKeyId(key_ids[0], -1, keyboard.max_key_id);
            CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);
            PrintKeyId(key_ids[0]);
            Sleep(40);
        }
        else if (ch == 'd' || ch == 'l' || ch == '.') { // NEXT (d/l/period)
            key_ids[0] = IncrementKeyId(key_ids[0], 1, keyboard.max_key_id);
            CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);
            PrintKeyId(key_ids[0]);
            Sleep(40);
        }
        else if (ch == ' ') { // print packet buffer
            CallPrintMessagesInBuffer(messages_sent, keyboard.BULK_LED_VALUE_MESSAGES_COUNT, keyboard.MESSAGE_LENGTH);
            Sleep(200);
        }
        else if (ch == 'q' || ch == 27) { // quit (q or ESC)
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return 0;
        }
        Sleep(10);
#endif
    }

    delete[] messages_sent;
	return 0;
}


}
