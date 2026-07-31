#include "keyboards/keyboard_spec_factory.h"

#include <stdexcept>
#include <string>

#include "misc.h"
#include "keyboards/abstract_keyboard.h"
#include "keyboards/rk84/rk84.h"
#include "keyboards/sk80/sk80.h"

namespace keylt {


AbstractKeyboard* KeyboardSpecFactory::CreateKeyboardSpec(KeyboardModel keyboard_model, Keyboard* keyboard_manager) {
    switch (keyboard_model) {
    case kRK84:
        return new rk84::RK84(keyboard_manager);
    case kSK80:
        return new sk80::SK80(keyboard_manager);
    }

    throw std::invalid_argument("Device not registered: keyboard_model " +
                                std::to_string(static_cast<int>(keyboard_model)));
}


}
