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

extern "C" DLL_EXPORT void HelloDll();
extern "C" DLL_EXPORT int BlinkKeys(char* keyIds, int nKeys);

#endif // BLINK_H
