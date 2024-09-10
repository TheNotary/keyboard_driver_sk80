#ifndef BLINK_H
#define BLINK_H

#ifdef _WIN32
#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif

#include <vector>
#include <string>
#include "misc.h"

extern "C" {
	DLL_EXPORT void PrintMessagesInBufferA(const unsigned char* buffer, size_t message_count, size_t message_length);
	DLL_EXPORT void HelloDll();
	DLL_EXPORT int BlinkKeys(char* keyIds, int nKeys);
	DLL_EXPORT int TurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char* messages_sent, KeyboardInfo keyboard);
	DLL_EXPORT int TurnOnKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard);
	DLL_EXPORT int TurnOffKeyIds(char* key_ids, UINT8 n_keys, KeyboardInfo keyboard);
	DLL_EXPORT int TurnOnKeyNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard);
	DLL_EXPORT int TurnOnOffNames(const std::vector<std::string>& key_names, KeyboardInfo keyboard);
	DLL_EXPORT int ListAvailableKeyboards(KeyboardInfo** out_keyboards);
	DLL_EXPORT void FreeKeyboards(KeyboardInfo* keyboards);
}

#endif // BLINK_H
