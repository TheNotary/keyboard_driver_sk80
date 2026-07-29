#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "misc.h"

// Thin C++ conveniences over the library's C ABI.
//
// The demo deliberately does not link against blink; it resolves the shared
// library at run time, which is the same thing a foreign-language consumer
// does. These helpers keep the dlopen/LoadLibrary bookkeeping out of the
// activity code.
namespace demo {

// Each of these opens the keyboard, performs one operation and closes it again.
// They return 0 on success and 1 on failure; LastLoaderError() then holds the
// detail reported by the library.
int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names, blink::KeyboardInfo keyboard);
int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names, blink::KeyboardInfo keyboard);

// messages_sent must have room for
// keyboard.BULK_LED_VALUE_MESSAGES_COUNT * keyboard.MESSAGE_LENGTH bytes.
int CallTurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char* messages_sent, blink::KeyboardInfo keyboard);

void CallPrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length);

std::vector<blink::KeyboardInfo> ListAvailableKeyboards();

// The message recorded by the most recent failing call, or an empty string.
std::string LastLoaderError();

}
