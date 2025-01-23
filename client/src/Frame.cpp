#include "../include/Frame.h"
#include <stdexcept>
#include <string>
#include <stdexcept>

using namespace std;

// Constructor
Frame::Frame(std::string cmd, std::unordered_map<std::string, std::string> hdrs, std::string bdy)
    : command(std::move(cmd)), headers(std::move(hdrs)), body(std::move(bdy)) {}

// add distructor 


// Set the command of the frame
void Frame::setCommand(const std::string& cmd) {
    command = cmd;
}

// Add or update a header
void Frame::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

// Set the body of the frame
void Frame::setBody(const std::string& bdy) {
    body = bdy;
}

// Get the command of the frame
std::string Frame::getCommand() const {
    return command;
}

// Change this 
// Get a header value by key
std::string Frame::getHeader(const std::string& key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

// Get the body of the frame
std::string Frame::getBody() const {
    return body;
}

// Serialize the frame to a STOMP-formatted string
std::string Frame::toString() const {
    std::ostringstream frame;
    frame << command << "\n";
    for (const auto& [key, value] : headers) {
        frame << key << ":" << value << "\n";
    }
    frame << "\n" << body << "\0"; // Null terminator
    return frame.str();
}

// Parse a frame from a STOMP-formatted string
Frame Frame::fromString(const std::string& rawFrame) {
    istringstream stream(rawFrame);
    string line;

    // Parse the command (first line)
    if (!getline(stream, line) || line.empty()) {
        throw std::invalid_argument("Invalid STOMP frame: missing command");
    }
    std::string cmd = line;

    // Parse headers
    unordered_map<std::string, std::string> hdrs;
    while (std::getline(stream, line) && !line.empty()) {
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            throw std::invalid_argument("Invalid STOMP frame: malformed header");
        }
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        hdrs[key] = value;
    }

    // Parse body
    string bdy;
    getline(stream, bdy, '\0'); // Read until null terminator

    return Frame(cmd, hdrs, bdy);
}
