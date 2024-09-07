#include <vector>
#include "misc.h"

std::vector<KeyboardInfo> known_keyboards = {
    {
        KeyboardModel::kRK84,
        0x258a,
        0x00c0,
        "RK84"
    },
    {
        KeyboardModel::kSK80,
        0x0000,
        0x0001,
        "SK80"
    },
};