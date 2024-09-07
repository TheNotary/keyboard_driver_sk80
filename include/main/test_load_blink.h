#pragma once
#include <vector>
#include "misc.h"

int test_dll(char* keyIds, int nKeys);
int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names);
int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names);
int CallTurnOnKeyIds(char* key_ids, UINT8 n_keys);
int CallTurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char messagesSent[3][65], KeyboardInfo keyboard);
void CallPrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length);
std::vector<KeyboardInfo> _ListAvailableKeyboards();

typedef void (*HelloDllFunc)();
typedef int  (*BlinkKeysFunc)(char* keyIds, int nKeys);
typedef int  (*TurnOnKeyNamesFunc)(const std::vector<std::string>& key_names);
typedef int  (*TurnOffKeyNamesFunc)(const std::vector<std::string>& key_names);
