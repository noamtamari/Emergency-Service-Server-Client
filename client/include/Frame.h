#pragma once

#include <string>
#include <unordered_map>
#include <sstream>

class Frame {
private:
    std::string command; // STOMP command (e.g., CONNECT, SUBSCRIBE, SEND, etc.)
    std::unordered_map<std::string, std::string> headers; // Headers as key-value pairs
    std::string body; // Optional message body

public:
    // Constructors
    Frame() = default;
    Frame(std::string cmd, std::unordered_map<std::string, std::string> hdrs, std::string bdy);

    // Setters
    void setCommand(const std::string& cmd);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& bdy);

    // Getters
    std::string getCommand() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;

    // Serialize frame to a string
    std::string toString() const;

    // Parse a frame from a string
    static Frame fromString(const std::string& rawFrame);
};
