#include <iostream>
#include <string>

#include "util.h"


namespace demo {

int ChooseActivity() {
    std::cout << "What activity would you like to start?" << std::endl;

    std::cout << "1. CycleKeyIds" << std::endl;
    std::cout << "2. PushToLight" << std::endl;
    std::cout << "3. Test esc Key" << std::endl;

    std::cout << std::endl << "Press 1, 2 or 3: ";

    return ParseNumericChoiceFromStdin();
}

}
