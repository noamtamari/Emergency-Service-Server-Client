#include "../include/keyboardInput.h"
#include <sstream>
#include <vector>
#include <string>

/**
 * @brief Parses a line of input into individual arguments.
 *
 * This function takes a single string input, typically from a keyboard or command line, 
 * and splits it into a vector of arguments. Each argument is separated by spaces in the input line.
 * 
 * @param line The input string containing arguments separated by spaces.
 * @return A vector of strings where each element is an argument extracted from the input line.
 * 
 * Example:
 * Input: "join channel1 user1"
 * Output: {"join", "channel1", "user1"}
 */
std::vector<std::string> keyboardInput::parseArguments(const std::string& line) {
    std::vector<std::string> arguments; // Vector to store individual arguments.
    std::istringstream stream(line); // Create a string stream from the input line.
    std::string argument;   // Temporary variable to hold each extracted argument.

    // Extract arguments separated by spaces and add them to the vector.
    while (stream >> argument) {
        arguments.push_back(argument);
    }

    // Return the vector containing all parsed arguments.
    return arguments;
}