#pragma once

#ifdef _WIN32
  #include <windows.h>
#else
  #include <cstdint>
  typedef uint8_t UINT8;
  typedef unsigned char UCHAR;
#endif

// Platform-neutral device handle type.
// On Windows, HANDLE is void* so this is ABI-compatible.
// On Linux, holds hid_device* from hidapi.
using DeviceHandle = void*;
