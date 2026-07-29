// Minimal downstream consumer of the installed blink package.
//
// This deliberately exercises only the one entry point that touches no USB
// hardware: CI runners have no keyboard attached, and the point of this program
// is to prove that the exported headers compile and that the exported symbols
// link and resolve at load time.

#include <cstdio>

#include <blink/blink.h>

int main() {
    const unsigned char empty[1] = {0};

    // A message count of zero prints nothing and opens no device.
    blinkdll::PrintMessagesInBufferA(empty, 0, 0);

    std::printf("blink package linked and loaded successfully\n");
    return 0;
}
