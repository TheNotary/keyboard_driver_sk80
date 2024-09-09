#include "keyboards/keyboard_spec_factory.h"

#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/rk84.h"
#include "keyboards/sk80/sk80.h"


AbstractKeyboard* KeyboardSpecFactory::CreateKeyboardSpec(KeyboardModel keyboard_model, Keyboard* keyboard_manager) {
    switch (keyboard_model) {
    case kRK84:
        return new rk84::RK84(keyboard_manager);
    case kSK80:
        return new sk80::SK80(keyboard_manager);
    }

    throw("Device not registered");
    return nullptr;
}
