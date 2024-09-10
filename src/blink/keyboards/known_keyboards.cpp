#include "keyboards/known_keyboards.h"

#include <vector>

#include "misc.h"
#include "keyboards/rk84/constants_rk84.h"
#include "keyboards/sk80/constants_sk80.h"

namespace blink {


// TODO:  Make it so this file isn't needed and derrivations of all the keyboards can be searched

std::vector<blink::KeyboardInfo> known_keyboards = {
    {
        blink::KeyboardModel::kRK84,
        0x258a,
        0x00c0,
        "RK84",
        rk84::max_key_id,
        rk84::MESSAGE_LENGTH,
        rk84::BULK_LED_VALUE_MESSAGES_COUNT
    },
    {
        blink::KeyboardModel::kSK80,
        0x05ac,
        0x024f,
        "SK80",
        sk80::max_key_id,
        sk80::MESSAGE_LENGTH,
        sk80::BULK_LED_VALUE_MESSAGES_COUNT
    },
};


}
