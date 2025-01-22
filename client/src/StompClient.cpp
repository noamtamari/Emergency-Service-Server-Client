#include "../include/ConnectionHandler.h"
#include <thread>
#include <iostream>
#include "../include/Frame.h"
#include "../include/keyboardInput.h"
#include <vector>
#include <string>
#include <regex>
#include <unordered_map>
#include "../include/event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events


using namespace std;
int reciepts = 0;
int subscription_id = 0;
bool connected = false;
unordered_map<string,int> channel_subscription = {};
ConnectionHandler *connectionHandler; // delete it somewhere do not forget 


int main(int argc, char *argv[]) {

    bool running = true;
    while (running){
        // how do you know this 
        std::string line;
        std::getline(std::cin, line);
        vector<string> read = keyboardInput::parseArguments(line);
        if (!connected && read[0] == "login"){
            std::regex pattern(R"(^[a-zA-Z0-9]+:[a-zA-Z0-9]+$)");
            bool hostPort = std::regex_match(read[1], pattern);
            if (!hostPort){
                std::cout << "Invalid input" << std::endl;
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
                if (!connectionHandler.connect()){
                    // change 
                    std::cout << "Cannot connect to " << host << ":" << portNum << std::endl;
                }
                else{
                    std::cout << "login succesful" << std::endl;
                    connected = true;
                    Frame frame("CONNECT", {{"accept-version", "1.2"}, {"login", read[2]}, {"passcode", read[3]}}, "");
                }
                // Send it 
            }
        }
        else if (connected && read[0] == "login"){
            std::cout << "" << std::endl;
        }
        else if (!connected && read[0] != "login"){
            std::cout << "please login first" << std::endl;
        }
        else{
            Frame* sendFrame = handleInput(read);
            // distruct frame 

        }
    }


	if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    
    // ConnectionHandler connectionHandler(host, port);
	//   if (!connectionHandler.connect()) {
    //     std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
    //     return 1;
    // }

    

    // std::thread serverThread(&connectionHandler);

	
	// TODO: implement the STOMP client
	return 0;
}

Frame *handleInput(vector<string> read){
    if (read[0] == "join"){
        return handleJoin(read);
    }
    else if (read[0] == "exit"){
        return handleExit(read);
    }
    else if (read[0] == "report"){
        return handleReport(read);
    }
    else if (read[0] == "logout"){
        return handleLogout(read);
    }
    else if (read[0] == "summery"){
        return handleSummery(read);
    }
    return nullptr;
}

Frame *handleLogout(vector<string> read){
    if (read.size() != 1){
        std::cout << "" << std::endl;
        return nullptr;
    }
    connected = false;
    std::cout << "Logged Out" << std::endl;
    return new Frame("DISCONNECT", {{"receipt", std::to_string(reciepts++)}},"");
}

Frame *handleJoin(vector<string> read){
    if (read.size() != 2){
        std::cout << "" << std::endl;
        return nullptr;
    }
    int join_subscription_id = subscription_id+1;
    channel_subscription.emplace(read[1], join_subscription_id);
    std::cout << "Joined channel " + read[1] << std::endl;
    return new Frame("SUBSCRIBE", {{"destination", read[1]},{"receipt", std::to_string(reciepts++)}, {"id", std::to_string(join_subscription_id)}},"");
}

Frame *handleExit(vector<string> read){
     if (read.size() != 2){
        std::cout << "" << std::endl;
        return nullptr;
    }
    auto channel_subscriptionId = channel_subscription.find(read[1]);
    if (channel_subscriptionId == channel_subscription.end()){
        std::cout << "you are not subscribed to channel " + channel_subscriptionId->first << std::endl;
        return nullptr;
    }
    std::cout << "Exited channel " + channel_subscriptionId->first << std::endl;
}

Frame *handleReport(vector<string> read){
    // Parse the events file specified in read[1]
    names_and_events information;
    try {
        information = parseEventsFile(read[1]);
        string channel = information.channel_name;
        std::vector<Event> events = information.events;
        for (Event& event : events) {
            // Create a frame for each event 
            event.setEventOwnerUser(connectionHandler->get_user_name());
            Frame frame("SEND", {{"destination", channel}, {"user", event.getEventOwnerUser()} , {"city", event.get_city()}
            , {"event name", event.get_name()}, {"date time", event.get_date_time()}
            ,{"general information", event.get_general_information()}
            ,{"description", event.get_description()}}, "");
            con
            // Send the frame
        }

    } catch (const std::exception& e) {
        std::cerr << "Error parsing events file: " << e.what() << std::endl;
        return nullptr;
    }
}

Frame *handleSummery(vector<string> read){
    // To do 

}



