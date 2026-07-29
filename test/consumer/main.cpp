// Minimal downstream consumer of the installed blink package.
//
// This deliberately exercises only entry points that touch no USB hardware: CI
// runners have no keyboard attached, and the point of this program is to prove
// that the exported header compiles and that the exported symbols link and
// resolve at load time.
//
// The C99 sibling (main.c) is what proves the header is callable from outside
// C++; this one proves it still works when included from C++.

#include <cstdio>

#include <blink/blink.h>

int main() {
    if (blink_abi_version() != BLINK_ABI_VERSION) {
        std::printf("ABI mismatch between header and library\n");
        return 1;
    }

    std::size_t known = 0;
    const blink_status status = blink_list_known_keyboards(nullptr, 0, &known);
    if (status != BLINK_OK && status != BLINK_ERR_BUFFER_TOO_SMALL) {
        std::printf("blink_list_known_keyboards failed: %s\n", blink_status_name(status));
        return 1;
    }

    std::printf("blink %s linked and loaded successfully; %u known models\n",
                blink_version_string(), static_cast<unsigned>(known));
    return 0;
}
