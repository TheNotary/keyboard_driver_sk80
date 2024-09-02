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

extern "C" DLL_EXPORT void HelloDll();
extern "C" DLL_EXPORT int BlinkKeys(char* keyIds, int nKeys);
extern "C" DLL_EXPORT int TurnOnKeyNames(const std::vector<std::string>& key_names);
extern "C" DLL_EXPORT int TurnOnOffNames(const std::vector<std::string>& key_names);

#endif // BLINK_H
