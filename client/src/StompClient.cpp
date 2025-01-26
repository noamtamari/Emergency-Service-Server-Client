#include "../include/ConnectionHandler.h"
#include <thread>
#include <iostream>
#include "../include/Frame.h"
#include "../include/keyboardInput.h"
#include "../include/StompProtocol.h"
#include <vector>
#include <string>
#include <regex>
#include <chrono>
#include <unordered_map>
#include "../include/Event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events
#include <list>
#include <chrono>

using namespace std;
unordered_map<string, int> channel_subscription = {};
ConnectionHandler *connectionHandler; // delete it somewhere do not forget

// Declare the serverListener function here
void serverListner(ConnectionHandler &connectionHandler, StompProtocol &stompProtocol, bool &running);

// Declare the isValidHostPort function here
bool isValidHostPort(const std::string &input);

int main(int argc, char *argv[])
{
    // bool connected = false;
    StompProtocol *stompProtocol = nullptr;
    ConnectionHandler *connectionHandler = nullptr;
    std::thread serverThread;

    bool running = true;
    while (running)
    {
        cout << "In the while " << endl;
        // how do you know this
        std::string line;
        std::getline(std::cin, line);
        vector<string> read = keyboardInput::parseArguments(line);
        if (read[0] == "close")
        {
            running = false;
            if (stompProtocol != nullptr && !stompProtocol->isConnected())
            {
                delete stompProtocol;
                stompProtocol = nullptr;

                delete connectionHandler; // Free the allocated memory and close the connection
                connectionHandler = nullptr;
            }
        }
        else
        {
            if (connectionHandler == nullptr && read[0] == "login")
            {
                if (read.size() != 4)
                {
                    std::cout << "\033[95login command needs 3 args: {host:port} {user} {password}\033[0m" << std::endl;
                    continue;
                }
                else
                {
                    // std::regex pattern(R"(^[a-zA-Z0-9]+:[a-zA-Z0-9]+$)");
                    bool hostPort = isValidHostPort(read[1]);
                    if (!hostPort)
                    {
                        std::cout << "\033[95mhost:port are illegal\033[0m" << std::endl;
                    }
                    else
                    {
                        string host;
                        string port;
                        int portNum;
                        // Find the position of the colon
                        size_t dotSpace = read[1].find(':');
                        // Extract host (string1) and port (string2)
                        host = read[1].substr(0, dotSpace);
                        port = read[1].substr(dotSpace + 1);
                        portNum = std::stoi(port);
                        connectionHandler = new ConnectionHandler(host, portNum, read[2]);
                        // see what happens with the connection handler and delete? try again , etc
                        if (!connectionHandler->connect())
                        {
                            std::cout << "\033[95mCould not connect to the server\033[0m" << std::endl;
                        }
                        else
                        {
                            stompProtocol = new StompProtocol(connectionHandler);
                            stompProtocol->handleLogin(read);
                            if (connectionHandler != nullptr && stompProtocol != nullptr && !stompProtocol->isConnected())
                            {
                                stompProtocol->setConnected(true);
                                //cout << "Starting server listener" << endl;
                                serverThread = std::thread(serverListner, std::ref(*connectionHandler), std::ref(*stompProtocol), std::ref(running));
                            }
                        }
                    }
                }
            }
            // Connection was not made and user wrote command that is not login
            else if (stompProtocol == nullptr && read[0] != "login")
            {
                std::cout << "\033[95mplease login first\033[0m" << std::endl;
            }
            // Connection was made but user tried to login again
            else if (stompProtocol != nullptr && read[0] == "login")
            {
                std::cout << "\033[95mThe client is already logged in, log out before trying again\033[0m" << std::endl;
            }
            // Connection was made and user tries to preform command that is not login
            else
            {
                cout << "User input: " << read[0] << endl;
                if (stompProtocol != nullptr)
                {
                    stompProtocol->processUserInput(read);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // delete both stomp protocol and connecntion hanler
            if ((stompProtocol != nullptr && !stompProtocol->isConnected()) || (read.size()==1 && read[0] == "logout"))
            {
                cout << "logouting!!! " << endl;
                serverThread.join();
            }
            if (stompProtocol != nullptr && !stompProtocol->isConnected())
            {
                delete stompProtocol;
                stompProtocol = nullptr;

                delete connectionHandler; // Free the allocated memory and close the connection
                connectionHandler = nullptr;
            }
        }
    }
    cout << "Exiting main" << endl;
    return 0;
}

void serverListner(ConnectionHandler &conncectionHandler, StompProtocol &stompProtocol, bool &running)
{
    std::list<string> msgs;
    while (stompProtocol.isConnected())
    {
        // cout << "Server listener running" << endl;
        string serverMessage;
        bool gotMessage = conncectionHandler.getLine(serverMessage);
        if (gotMessage && !serverMessage.empty())
        {
            //cout << "Server message: " << serverMessage << endl;
            stompProtocol.processServerFrame(serverMessage);
        }
    }
}

bool isValidHostPort(const std::string &input)
{
    // Find the position of the colon
    size_t colonPos = input.find(':');

    // Ensure the colon exists and has characters on both sides
    return (colonPos != std::string::npos) && // Colon exists
           (colonPos > 0) &&                  // Characters before colon
           (colonPos < input.length() - 1);   // Characters after colon
}
