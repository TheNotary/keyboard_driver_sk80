#include <windows.h>
#include <iostream>
#include <vector>
#include "test_load_blink.h"

typedef void (*HelloDllFunc)();
typedef int (*BlinkKeysFunc)(char* keyIds, int nKeys);
typedef int (*BlinkKeyNamesFunc)(const std::vector<std::string>& key_names);

/*
* @brief A generic function for loading a function from a dll.
* 
*/
template<typename FuncType>
FuncType GetFunction(HMODULE* hModule, const char* funcName) {
    FuncType func = (FuncType)GetProcAddress(*hModule, funcName);
    if (!func) {
        std::cerr << "Failed to get function address: " << funcName << std::endl;
        return nullptr;
    }
    return func;
}

int test_dll(char* keyIds, int nKeys) {
    //
    // Load the DLL with the functions of interest
    //

    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }


    //
    // Get Functions from DLL
    //

    HelloDllFunc helloDll = GetFunction<HelloDllFunc>(&hModule, "HelloDll");
    if (!helloDll) {
        FreeLibrary(hModule);
        return 1;
    }

    BlinkKeysFunc BlinkKeys = GetFunction<BlinkKeysFunc>(&hModule, "BlinkKeys");
    if (!BlinkKeys) {
        FreeLibrary(hModule);
        return 1;
    }


    //
    // Invoke functions
    //

    helloDll();

    if (BlinkKeys(keyIds, nKeys) != 0) {
        std::cout << "Problem blinking keys" << std::endl;
    }

    FreeLibrary(hModule);

    return 0;
}


int CallDllBlinkKeyNames(const std::vector<std::string>& key_names) {
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }

    BlinkKeyNamesFunc BlinkKeyNames = GetFunction<BlinkKeyNamesFunc>(&hModule, "BlinkKeyNames");
    if (!BlinkKeyNames) {
        FreeLibrary(hModule);
        return 1;
    }

    return BlinkKeyNames(key_names);
}
