/*
 * Translation unit handed to bindgen. The installed layout puts the public
 * header at <prefix>/include/blink/blink.h, and the vendored CMake install
 * reproduces that layout in OUT_DIR, so a single include path works for every
 * discovery mode.
 */
#include <blink/blink.h>
