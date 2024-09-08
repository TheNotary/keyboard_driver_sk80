#include "keyboard.h"
#include "keyboards/keyboard_spec_factory.h"
#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/rk84.h"


AbstractKeyboard* KeyboardSpecFactory::CreateKeyboardSpec(KeyboardModel keyboard_model) {
    switch (keyboard_model) {
    case kRK84:
        return new rk84::RK84();
        //case kSK80:
        //    return new SK80();
    }

    return nullptr; // or handle with exceptions
}
