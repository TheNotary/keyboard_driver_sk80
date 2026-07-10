#include <iostream>

#include "misc.h"
#include "choose_keyboard.h"
#include "choose_activity.h"
#include "activities/push_to_light.h"
#include "activities/cycle_keyids.h"
#include "activities/test_single_key.h"

#ifndef _WIN32
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#endif


int main() {
#ifndef _WIN32
    int lock_fd = open("/tmp/keyboard_driver.lock", O_CREAT | O_RDWR, 0600);
    if (lock_fd < 0 || flock(lock_fd, LOCK_EX | LOCK_NB) != 0) {
        std::cerr << "Another instance is already running." << std::endl;
        return 1;
    }
#endif

    blink::KeyboardInfo keyboard = demo::ChooseKeyboard();
    if (keyboard.display_name[0] == 0x00) {
        std::cout << "Exiting, no valid keyboard selected" << std::endl;
        return 0;
    }

    int activity = demo::ChooseActivity();  // 1 == CycleKeyIds

    if (activity == 1) {
        demo::CycleKeyIds(keyboard);
        return 0;
    }
    if (activity == 2) {
        demo::PushToLight(keyboard);
        return 0;
    }
    if (activity == 3) {
        demo::TestSingleKey(keyboard);
        return 0;
    }

    std::cout << "Invalid choice." << std::endl;
    return 0;
    
}
