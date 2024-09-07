#include <windows.h>
#include <iostream>
#include <vector>
#include "test_load_blink.h"

using std::cout;
using std::endl;


void PrintKeyId(char key_id) {
    std::cout << "\r" << std::string(12, ' ');
    std::cout << "\r" << "KeyId: " << (int)key_id;
}

char IncrementKeyId(char value, int incrementation) {
    const UINT8 max_index = 96;

    // if we don't cast result to unsigned, it screws up the '>' comparisons, 
    // making 0xff -1 instead of 255!
    // We don't need to return unsigned char since the calling code does implicit cast
    // If we pass in value = 0xff, isn't that going to be a different computation 
    // whether we're signed or unsigned???

    unsigned char result = ((unsigned)value) + incrementation; 

    if (result > max_index)  // If we're incrementing passed max_index
        return 1;            // then overflow the char to 1 after incrementing max_index
    
    if (result == 0)       // It is only possible to reach 0 if we're incrementing by -1
        return max_index;  // Therefore overflow to max_index

    return result;
}

int CycleKeyIds(KeyboardInfo keyboard) {
    cout << "CycleKeyIds Debug Mode:" << endl
        << "The key id will be shown on the screen, and the LED for that key will be switched "
        << "on making mapping the keyboard easy.  " 
        << endl << endl
        << "Press left or right to cycle through the keyId to test.  " << endl
        << "Press Space to print the buffer that was last transmitted to the keyboard" << endl
        << "Press escape to exit" 
        << endl << endl;

    //std::vector<std::string> key_names = { "f11" };
    unsigned char messages_sent[3][65] = { 0 };
    messages_sent[0][0] = { 2 }; // debugging, delete me
    char key_ids[] = { 0x01 };

    PrintKeyId(key_ids[0]);
    CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);

    while (true) {
        if (GetAsyncKeyState(VK_DOWN) & 0x8000
            || GetAsyncKeyState(VK_LEFT) & 0x8000) { // PREV
            key_ids[0] = IncrementKeyId(key_ids[0], -1); 
            CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);
            PrintKeyId(key_ids[0]);

            Sleep(40); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000
            || GetAsyncKeyState(VK_RIGHT) & 0x8000) { // NEXT
            key_ids[0] = IncrementKeyId(key_ids[0], 1);
            CallTurnOnKeyIdsD(key_ids, sizeof(key_ids), messages_sent, keyboard);
            PrintKeyId(key_ids[0]);

            Sleep(40); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {  // print packet buffer
            CallPrintMessagesInBuffer(*messages_sent, 3, 65);

            Sleep(200); // Simple debounce delay
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return 0;
        }

        Sleep(10);
    }
	return 0;
}