#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iostream>


class keyboardInput {
    public:
        static std::vector<std::string> parseArguments(const std::string& line);
};