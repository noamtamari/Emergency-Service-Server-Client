#include "../include/ConnectionHandler.h"
#include <thread>
#include <iostream>
#include "../include/Frame.h"
#include "../include/keyboardInput.h"
#include "../include/StompProtocol.h"
#include <vector>
#include <string>
#include <regex>
#include <unordered_map>
#include "../include/event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events


using namespace std;
bool connected = true;
unordered_map<string,int> channel_subscription = {};
ConnectionHandler *connectionHandler; // delete it somewhere do not forget 


int main(int argc, char *argv[]) {
    StompProtocol *stompProtocol = nullptr;
    ConnectionHandler *connectionHandler = nullptr;
    bool running = true;
    while (running){
        // how do you know this 
        std::string line;
        std::getline(std::cin, line);
        vector<string> read = keyboardInput::parseArguments(line);
        if (stompProtocol == nullptr && read[0] == "login"){
            std::regex pattern(R"(^[a-zA-Z0-9]+:[a-zA-Z0-9]+$)");
            bool hostPort = std::regex_match(read[1], pattern);
            if (!hostPort){
                std::cout << "host:port are illegal" << std::endl;
            }
            else{
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
                if (!connectionHandler->connect()){
                    std::cout << "Cannot connect to the server" << std::endl;
                }
                else{
                    stompProtocol = new StompProtocol(connectionHandler);
                    stompProtocol->handleLogin(read);
                }
            }
        }
        // Connection was not made and user wrote command that is not login
        else if (stompProtocol == nullptr && read[0] != "login"){
            cout << "please login first" << std::endl;
        }
        // Connection was made but user tried to login again
        else if (stompProtocol != nullptr && read[0] == "login"){
            cout << "user already logedin " << std::endl;
        }
        // Connection was made and user tries to preform command that is not login 
        else{
            if (stompProtocol != nullptr){
                stompProtocol->processUserInput(read);
            }
        }
        if (connectionHandler != nullptr){
            std::thread serverThread(serverListner, connectionHandler, stompProtocol, &running);
        }
        // delete both stomp protocol and connecntion hanler 
    }
    if (stompProtocol != nullptr){
        delete stompProtocol;
    }
    if (connectionHandler != nullptr){
        delete connectionHandler;
    }
	return 0;
}

void serverListner(ConnectionHandler &conncectionHandler, StompProtocol &stompProtocol,bool &running){
    while(running){
        string serverMessage;
        bool gotMessage = conncectionHandler.getLine(serverMessage);
        if (gotMessage){
            stompProtocol.processServerFrame(serverMessage);
        }
        // more things 
    }
}




