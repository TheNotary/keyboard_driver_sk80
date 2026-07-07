#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include <vector>
#include "blink_loader.h"
#include "misc.h"


namespace demo {

using namespace blink;

#ifdef _WIN32
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
#else
// Linux shared library handle wrapper
static void* g_lib_handle = nullptr;

static void* LoadSharedLib() {
    if (!g_lib_handle) {
        g_lib_handle = dlopen("libblink.so", RTLD_LAZY);
        if (!g_lib_handle) {
            std::cerr << "Failed to load shared library: " << dlerror() << std::endl;
        }
    }
    return g_lib_handle;
}

template<typename FuncType>
FuncType GetFunction(void* handle, const char* funcName) {
    FuncType func = (FuncType)dlsym(handle, funcName);
    if (!func) {
        std::cerr << "Failed to get function address: " << funcName << " - " << dlerror() << std::endl;
        return nullptr;
    }
    return func;
}
#endif


void CallPrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length) {
#ifdef _WIN32
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return;
    }

    char funcName[] = "PrintMessagesInBufferA";
    typedef void (*Func)(unsigned char* buffer, size_t message_count, size_t message_length);

    Func dll_func = GetFunction<Func>(&hModule, funcName);
    if (!dll_func) {
        FreeLibrary(hModule);
        return;
    }

    dll_func(buffer, message_count, message_length);
#else
    void* handle = LoadSharedLib();
    if (!handle) return;

    typedef void (*Func)(unsigned char* buffer, size_t message_count, size_t message_length);
    Func dll_func = GetFunction<Func>(handle, "PrintMessagesInBufferA");
    if (!dll_func) return;

    dll_func(buffer, message_count, message_length);
#endif
}

int CallTurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char* messagesSent, KeyboardInfo keyboard) {
#ifdef _WIN32
    HMODULE hModule = LoadLibrary(TEXT("blink.dll"));
    if (!hModule) {
        std::cerr << "Failed to load DLL" << std::endl;
        return 1;
    }

    char funcName[] = "TurnOnKeyIdsD";
    typedef int  (*Func)(char* key_ids, UINT8 n_keys, unsigned char* messagesSent, KeyboardInfo keyboard);

    Func dll_func = GetFunction<Func>(&hModule, funcName);
    if (!dll_func) {
        FreeLibrary(hModule);
        return 1;
    }

    return dll_func(key_ids, n_keys, messagesSent, keyboard);
#else
    void* handle = LoadSharedLib();
    if (!handle) return 1;

    typedef int (*Func)(char* key_ids, UINT8 n_keys, unsigned char* messagesSent, KeyboardInfo keyboard);
    Func dll_func = GetFunction<Func>(handle, "TurnOnKeyIdsD");
    if (!dll_func) return 1;

    return dll_func(key_ids, n_keys, messagesSent, keyboard);
#endif
}


int CallTurnOnKeyIds(char* key_ids, UINT8 n_keys) {
#ifdef _WIN32
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
#else
    void* handle = LoadSharedLib();
    if (!handle) return 1;

    typedef int (*Func)(char* key_ids, UINT8 n_keys);
    Func dll_func = GetFunction<Func>(handle, "TurnOnKeyIds");
    if (!dll_func) return 1;

    return dll_func(key_ids, n_keys);
#endif
}

std::vector<blink::KeyboardInfo> ListAvailableKeyboards() {
    std::vector<KeyboardInfo> list;

#ifdef _WIN32
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
    char funcNameFree[] = "FreeKeyboards";
    typedef void (*FuncFree)(KeyboardInfo* keyboards);

    FuncFree FreeKeyboards = GetFunction<FuncFree>(&hModule, funcNameFree);
    FreeKeyboards(keyboards);
#else
    void* handle = LoadSharedLib();
    if (!handle) return list;

    typedef int (*Func)(KeyboardInfo** out_keyboards);
    Func listFunc = GetFunction<Func>(handle, "ListAvailableKeyboards");
    if (!listFunc) return list;

    KeyboardInfo* keyboards = nullptr;
    int n_keyboards = listFunc(&keyboards);
    for (int i = 0; i < n_keyboards; i++) {
        list.push_back(keyboards[i]);
    }

    typedef void (*FuncFree)(KeyboardInfo* keyboards);
    FuncFree freeFunc = GetFunction<FuncFree>(handle, "FreeKeyboards");
    if (freeFunc) freeFunc(keyboards);
#endif

    return list;
}

int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
#ifdef _WIN32
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

    return TurnOnKeyNames(key_names, keyboard);
#else
    void* handle = LoadSharedLib();
    if (!handle) return 1;

    TurnOnKeyNamesFunc TurnOnKeyNames = GetFunction<TurnOnKeyNamesFunc>(handle, "TurnOnKeyNames");
    if (!TurnOnKeyNames) return 1;

    return TurnOnKeyNames(key_names, keyboard);
#endif
}

int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard) {
#ifdef _WIN32
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

    return TurnOffKeyNames(key_names, keyboard);
#else
    void* handle = LoadSharedLib();
    if (!handle) return 1;

    TurnOffKeyNamesFunc TurnOffKeyNames = GetFunction<TurnOffKeyNamesFunc>(handle, "TurnOnOffNames");
    if (!TurnOffKeyNames) return 1;

    return TurnOffKeyNames(key_names, keyboard);
#endif
}

}
