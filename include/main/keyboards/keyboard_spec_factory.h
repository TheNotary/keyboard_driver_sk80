#pragma once
#include "misc.h"
#include "keyboards/abstract_keyboard.h"

namespace blink {


class KeyboardSpecFactory {
public:
    AbstractKeyboard* CreateKeyboardSpec(KeyboardModel keyboard_model, Keyboard* keyboard_manager);
};


}
