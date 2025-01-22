#pragma once
#include <string>
#include "../include/Frame.h"
using namespace std;

class StompEncoderDecoder {
    private: 
    public: 
        // Encode a frame to a string
        string encode(const Frame& frame) const;

        // Decode a frame from a string
        Frame decode(const std::string& rawFrame) const;
}