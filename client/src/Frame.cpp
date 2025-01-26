#include "../include/Frame.h"
#include <stdexcept>
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

/**
 * @brief Constructor for the Frame class.
 *
 * @param cmd The command of the STOMP frame (e.g., CONNECT, SUBSCRIBE, etc.).
 * @param hdrs A map containing headers of the frame (key-value pairs).
 * @param bdy The body of the frame (if any).
 */
Frame::Frame(std::string cmd, std::unordered_map<std::string, std::string> hdrs, std::string bdy)
    : command(std::move(cmd)), headers(std::move(hdrs)), body(std::move(bdy)) {}


/**
 * @brief Sets the command of the frame.
 *
 * @param cmd The command to set (e.g., CONNECT, SUBSCRIBE, etc.).
 */
void Frame::setCommand(const std::string &cmd)
{
    command = cmd;
}

/**
 * @brief Adds or updates a header in the frame.
 *
 * @param key The header key.
 * @param value The header value.
 */
void Frame::setHeader(const std::string &key, const std::string &value)
{
    headers[key] = value;
}

/**
 * @brief Sets the body of the frame.
 *
 * @param bdy The body string to set.
 */
void Frame::setBody(const std::string &bdy)
{
    body = bdy;
}


/**
 * @brief Gets the command of the frame.
 *
 * @return The command of the frame.
 */
std::string Frame::getCommand() const
{
    return command;
}

/**
 * @brief Gets the value of a specific header.
 *
 * @param key The header key.
 * @return The value of the header, or an empty string if the key is not found.
 */
std::string Frame::getHeader(const std::string &key) const
{
    auto it = headers.find(key);
    if (it != headers.end())
    {
        return it->second;
    }
    return "";
}

/**
 * @brief Gets the body of the frame.
 *
 * @return The body of the frame.
 */
std::string Frame::getBody() const
{
    return body;
}

/**
 * @brief Serializes the frame into a STOMP-formatted string.
 *
 * This method converts the frame's components (command, headers, and body) into a
 * single string that adheres to the STOMP protocol format.
 *
 * @return A string representation of the frame in STOMP format.
 */
std::string Frame::toString() const
{
    std::ostringstream frame;
    frame << command << "\n";
    for (const auto &pair : headers)  // Add each header as a key:value pair.
    {
        const auto &key = pair.first;
        const auto &value = pair.second;
        frame << key << ":" << value << "\n";
    }
    frame << "\n"
          << body << "\0"; // Add body and null terminator at the end.
    return frame.str();
}

/**
 * @brief Parses a raw STOMP-formatted string into a Frame object.
 *
 * This method splits the raw string into its components (command, headers, and body)
 * and creates a Frame object.
 *
 * @param rawFrame The input string in STOMP format.
 * @return A Frame object representing the parsed input.
 * @throws std::invalid_argument If the input is malformed or missing components.
 */
Frame Frame::fromString(const std::string &rawFrame)
{
    istringstream stream(rawFrame); // Create a string stream from the input.

    string line;

     // Parse the command (first line).
    if (!getline(stream, line) || line.empty())
    {
        throw std::invalid_argument("Invalid STOMP frame: missing command");
    }
    std::string cmd = line;

    // Parse headers
    unordered_map<std::string, std::string> hdrs;
    while (std::getline(stream, line) && !line.empty())
    {
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos)
        {
            throw std::invalid_argument("Invalid STOMP frame: malformed header");
        }
        std::string key = line.substr(0, colonPos); // Extract header key
        std::string value = line.substr(colonPos + 1); // Extract header value.
        if (value[0] == ' ') // Remove leading space in value if present.
            std::string value = value.substr(1, value.length() + 1);
        hdrs[key] = value;
    }

    // Parse body
    string bdy;
    getline(stream, bdy, '\0'); // Read until null terminator

    return Frame(cmd, hdrs, bdy); // Return a constructed Frame object.
}
