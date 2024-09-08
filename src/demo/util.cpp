#include <iostream>
#include <string>


namespace demo {

int ParseNumericChoiceFromStdin() {
    std::string line;
    std::getline(std::cin, line);

    int choice;

    try {
        choice = std::stoi(line);
    }
    catch (...) {
        std::cerr << "Error: Invalid input, specify a number." << std::endl;
        return -1;
    }

    return choice;
}

}
