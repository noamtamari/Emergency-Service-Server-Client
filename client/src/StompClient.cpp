#include "../include/ConnectionHandler.h"
#include "../include/Frame.h"
#include "../include/keyboardInput.h"
#include "../include/StompProtocol.h"
#include "../include/Event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events

#include <string>
#include <chrono>
#include <list>
#include <thread>
#include <iostream>

using namespace std;

// Declare the serverListener function here
void serverListner(ConnectionHandler &connectionHandler, StompProtocol &stompProtocol);

// Declare the isValidHostPort function here
bool isValidHostPort(const std::string &input);

int main(int argc, char *argv[])
{
    StompProtocol *stompProtocol = nullptr; // Pointer to handle STOMP protocol operations
    ConnectionHandler *connectionHandler = nullptr;  // Pointer to connectionHandler
    std::thread serverThread;  // Thread for server listener

    bool running = true; // Main loop control variable
    
    while (running)
    {
        // Read user input from the terminal
        std::string line;
        std::getline(std::cin, line); // Reads one line of input from the terminal
        vector<string> read = keyboardInput::parseArguments(line); // Parses the input into a vector of strings

        // Handle the "close" command to terminate the client
        if (read.size() != 0 && read[0] == "close")
        {
            running = false; // Breaks out of the main loop

            // Ensures the server listener thread is terminated properly
            if (serverThread.joinable()) {
                    serverThread.join(); // Wait for the server listener thread to terminate
            }

            // Clean up dynamically allocated resources
            if (stompProtocol != nullptr && !stompProtocol->isConnected())
            {
                delete stompProtocol;
                stompProtocol = nullptr;
            }
            if (connectionHandler != nullptr){
                delete connectionHandler; 
                connectionHandler = nullptr;
            }
        }
        else
        {
            // Handle "login" command to establish a connection
            if (connectionHandler == nullptr && read.size() != 0 && read[0] == "login")
            {
                if (read.size() != 4)
                {
                    std::cout << "\033[95mlogin command needs 3 args: {host:port} {user} {password}\033[0m" << std::endl;
                    continue;
                }
                else
                {
                    // Validate the format of the host:port
                    bool hostPort = isValidHostPort(read[1]);
                    if (!hostPort)
                    {
                        std::cout << "\033[95mhost:port are illegal\033[0m" << std::endl;
                    }
                    else
                    {
                        // Extract host and port from the input
                        string host;
                        string port;
                        int portNum;
                        size_t dotSpace = read[1].find(':');

                        // Extract host (string1) and port (string2)
                        host = read[1].substr(0, dotSpace);
                        port = read[1].substr(dotSpace + 1);
                        portNum = std::stoi(port);
                        // Create a new connection handler and attempt to connect
                        connectionHandler = new ConnectionHandler(host, portNum, read[2]);
                        if (!connectionHandler->connect())
                        {
                            std::cout << "\033[95mCould not connect to the server\033[0m" << std::endl;
                            delete connectionHandler; // Clean up on failure
                            connectionHandler = nullptr;
                        }
                        else
                        {
                            // Create a new STOMP protocol handler
                            stompProtocol = new StompProtocol(connectionHandler);
                            stompProtocol->handleLogin(read);
                            
                            // Start the server listener thread if the connection is successful
                            if (connectionHandler != nullptr && stompProtocol != nullptr && !stompProtocol->isConnected())
                            {
                                stompProtocol->setConnected(true);
                                serverThread = std::thread(serverListner, std::ref(*connectionHandler), std::ref(*stompProtocol));
                            }
                        }
                    }
                }
            }
             // Handle invalid or out-of-order commands

            // Connection was not made and user wrote command that is not login
            else if (stompProtocol == nullptr && (read.size() == 0 || read[0] != "login"))
            {
                std::cout << "\033[95mplease login first\033[0m" << std::endl;
            }
            // Connection was made but user tried to login again
            else if (stompProtocol != nullptr && (read.size() != 0 && read[0] == "login"))
            {
                std::cout << "\033[95mThe client is already logged in, log out before trying again\033[0m" << std::endl;
            }
            // Connection was made and user tries to preform command that is not login
            else
            {
                // Pass other commands to the STOMP protocol handler
                if (stompProtocol != nullptr)
                {
                    stompProtocol->processUserInput(read);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // delete both stomp protocol and connecntion hanler
            if ((stompProtocol != nullptr && !stompProtocol->isConnected()) || (read.size() == 1 && connectionHandler != nullptr && read[0] == "logout"))
            {
                cout << "logouting!!! " << endl;
                serverThread.join();
            }
            if (stompProtocol != nullptr && !stompProtocol->isConnected())
            {
                delete stompProtocol; // Free the allocated memory
                stompProtocol = nullptr;

                if (connectionHandler != nullptr){
                    delete connectionHandler; // Free the allocated memory and close the connection
                    connectionHandler = nullptr;
                }
            }
        }
    }
    // Program termination message
    cout << "Closing client.." << endl;
    return 0;
}

// Server listener function (runs in a separate thread)
void serverListner(ConnectionHandler &conncectionHandler, StompProtocol &stompProtocol)
{
    std::list<string> msgs;
    while (stompProtocol.isConnected())
    {
        ;
        string serverMessage;
        bool gotMessage = conncectionHandler.getLine(serverMessage);

         // Process valid messages received from the server
        if (gotMessage && !serverMessage.empty())
        {
            stompProtocol.processServerFrame(serverMessage);
        }
    }
}

// Validates host:port format
bool isValidHostPort(const std::string &input)
{
    // Find the position of the colon
    size_t colonPos = input.find(':');

    // Ensure the colon exists and has characters on both sides
    return (colonPos != std::string::npos) && // Colon exists
           (colonPos > 0) &&                  // Characters before colon
           (colonPos < input.length() - 1);   // Characters after colon
}
