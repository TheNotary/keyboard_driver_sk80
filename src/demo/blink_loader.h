#pragma once
#include <vector>
#include "misc.h"


namespace demo {

int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names, blink::KeyboardInfo keyboard);
int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names, blink::KeyboardInfo keyboard);
int CallTurnOnKeyIds(char* key_ids, UINT8 n_keys);
int CallTurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char* messagesSent, blink::KeyboardInfo keyboard);
void CallPrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length);
std::vector<blink::KeyboardInfo> ListAvailableKeyboards();

typedef void (*HelloDllFunc)();
typedef int  (*BlinkKeysFunc)(char* keyIds, int nKeys);
typedef int  (*TurnOnKeyNamesFunc)(const std::vector<std::string>& key_names, blink::KeyboardInfo keyboard);
typedef int  (*TurnOffKeyNamesFunc)(const std::vector<std::string>& key_names, blink::KeyboardInfo keyboard);

}
