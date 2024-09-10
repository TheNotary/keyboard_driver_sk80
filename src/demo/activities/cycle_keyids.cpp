#include "cycle_keyids.h"

#include <iostream>
#include <vector>

#include <windows.h>

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
        << "Press left or right to cycle through the keyId to test.  " << endl
        << "Press Space to print the buffer that was last transmitted to the keyboard" << endl
        << "Press escape to exit"
        << endl << endl;

    //std::vector<std::string> key_names = { "f11" };

    unsigned char* messages_sent = new unsigned char [keyboard.BULK_LED_VALUE_MESSAGES_COUNT * keyboard.MESSAGE_LENGTH];

    //unsigned char messages_sent[3][keyboard.MESSAGE_LENGTH] = { 0 };
    char key_ids[] = { 0x01 };

    PrintKeyId(key_ids[0]);
    CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);

    while (true) {
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
    }

    delete[] messages_sent;
	return 0;
}


}
