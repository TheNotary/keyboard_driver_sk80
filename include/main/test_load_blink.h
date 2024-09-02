#pragma once
#include <vector>

int test_dll(char* keyIds, int nKeys);
int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names);
int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names);
