#pragma once

#ifdef _WIN32
  // <windows.h> defines min/max as function-like macros unless NOMINMAX is set,
  // which breaks every std::min / std::max call in a translation unit that
  // reaches this header. Keep these before the include, and mirror them as
  // compile definitions in CMakeLists.txt for units that include <windows.h>
  // through some other header first.
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
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
