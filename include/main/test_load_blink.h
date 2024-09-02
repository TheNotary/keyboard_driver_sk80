#pragma once
#include <vector>

int test_dll(char* keyIds, int nKeys);
int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names);
int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names);

typedef void (*HelloDllFunc)();
typedef int  (*BlinkKeysFunc)(char* keyIds, int nKeys);
typedef int  (*TurnOnKeyNamesFunc)(const std::vector<std::string>& key_names);
typedef int  (*TurnOffKeyNamesFunc)(const std::vector<std::string>& key_names);
