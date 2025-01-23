#include "../include/keyboardInput.h"
#include <sstream>
#include <vector>
#include <string>

std::vector<std::string> keyboardInput::parseArguments(const std::string& line) {
    std::vector<std::string> arguments;
    std::istringstream stream(line);
    std::string argument;

    while (stream >> argument) {
        arguments.push_back(argument);
    }

    return arguments;
}