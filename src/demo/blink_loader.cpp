#include <windows.h>
#include <iostream>
#include <vector>
#include "blink_loader.h"
#include "misc.h"


namespace demo {

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


void CallPrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length) {
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return;
    }

    char funcName[] = "PrintMessagesInBuffer";
    typedef void (*Func)(unsigned char* buffer, size_t message_count, size_t message_length);

    Func dll_func = GetFunction<Func>(&hModule, funcName);
    if (!dll_func) {
        FreeLibrary(hModule);
        return;
    }

    dll_func(buffer, message_count, message_length);
}

int CallTurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char messagesSent[3][65], KeyboardInfo keyboard) {
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }

    char funcName[] = "TurnOnKeyIdsD";
    typedef int  (*Func)(char* key_ids, UINT8 n_keys, unsigned char messagesSent[3][65], KeyboardInfo keyboard);

    Func dll_func = GetFunction<Func>(&hModule, funcName);
    if (!dll_func) {
        FreeLibrary(hModule);
        return 1;
    }

    return dll_func(key_ids, n_keys, messagesSent, keyboard);
}


int CallTurnOnKeyIds(char* key_ids, UINT8 n_keys) {
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }

    char funcName[] = "TurnOnKeyIds";
    typedef int  (*Func)(char* key_ids, UINT8 n_keys);

    Func dll_func = GetFunction<Func>(&hModule, funcName);
    if (!dll_func) {
        FreeLibrary(hModule);
        return 1;
    }

    return dll_func(key_ids, n_keys);
}

std::vector<KeyboardInfo> _ListAvailableKeyboards() {
    std::vector<KeyboardInfo> list;

    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return list;
    }

    char funcName[] = "ListAvailableKeyboards";
    typedef int (*Func)(KeyboardInfo** out_keyboards);

    Func ListAvailableKeyboards = GetFunction<Func>(&hModule, funcName);
    if (!ListAvailableKeyboards) {
        FreeLibrary(hModule);
        return list;
    }

    // Extract keyboards from the DLL's memory
    KeyboardInfo* keyboards = nullptr;
    int n_keyboards = ListAvailableKeyboards(&keyboards);
    for (int i = 0; i < n_keyboards; i++) {
        list.push_back(keyboards[i]);
    }

    // Clean up the DLL's memory
    // I'm not conviced this requires a DLL invocation

    char funcNameFree[] = "FreeKeyboards";
    typedef void (*FuncFree)(KeyboardInfo* keyboards);

    FuncFree FreeKeyboards = GetFunction<FuncFree>(&hModule, funcNameFree);
    FreeKeyboards(keyboards);

    return list;
}

int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names) {
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }

    TurnOnKeyNamesFunc TurnOnKeyNames = GetFunction<TurnOnKeyNamesFunc>(&hModule, "TurnOnKeyNames");
    if (!TurnOnKeyNames) {
        FreeLibrary(hModule);
        return 1;
    }

    return TurnOnKeyNames(key_names);
}

int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names) {
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }

    TurnOffKeyNamesFunc TurnOffKeyNames = GetFunction<TurnOffKeyNamesFunc>(&hModule, "TurnOnOffNames");
    if (!TurnOffKeyNames) {
        FreeLibrary(hModule);
        return 1;
    }

    return TurnOffKeyNames(key_names);
}

}
