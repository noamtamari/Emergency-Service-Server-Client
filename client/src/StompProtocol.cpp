#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include <string>
#include <iostream>
#include "../include/keyboardInput.h"
#include <vector>
#include <regex>
#include <unordered_map>
#include "../include/event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler &connectionHandler, StompEncoderDecoder encoderDecoder) :
connectionHandler(connectionHandler), encoderDecoder(encoderDecoder) {}

void StompProtocol::handleInput(vector<string> read){
    if (read[0] == "join"){
        handleJoin(read);
    }
    else if (read[0] == "exit"){
        handleExit(read);
    }
    else if (read[0] == "report"){
        handleReport(read);
    }
    else if (read[0] == "logout"){
        handleLogout(read);
    }
    else if (read[0] == "summery"){
        handleSummery(read);
    }
}

void StompProtocol::handleLogout(vector<string> read){
    if (read.size() != 1){
        std::cout << "" << std::endl;
    }
    else{
        connected = false;
        std::cout << "Logged Out" << std::endl;
        Frame frame("DISCONNECT", {{"receipt", std::to_string(reciepts++)}},"");
        string send = frame.toString();
        connectionHandler.sendLine(send);
    }
}

void StompProtocol::handleJoin(vector<string> read){
    if (read.size() != 2){
        cout << "" << endl;
    }
    else{
         int join_subscription_id = subscription_id+1;
        channel_subscription.emplace(read[1], join_subscription_id);
        cout << "Joined channel " + read[1] << endl;
        Frame frame("SUBSCRIBE", {{"destination", read[1]},{"receipt", std::to_string(reciepts++)}, {"id", std::to_string(join_subscription_id)}},"");
        string send = frame.toString();
        connectionHandler.sendLine(send);
    }
}

void StompProtocol::handleExit(vector<string> read){
     if (read.size() != 2){
        std::cout << "" << std::endl;
    }
    else{
        auto channel_subscriptionId = channel_subscription.find(read[1]);
        if (channel_subscriptionId == channel_subscription.end()){
            cout << "you are not subscribed to channel " + channel_subscriptionId->first << endl;
        }
        else{
            cout << "Exited channel " + channel_subscriptionId->first << endl;
            Frame frame("UNSUBSCRIBE", {{"destination", read[1]},{"receipt", to_string(reciepts++)}, {"id", to_string(channel_subscriptionId->second)}},"");
            string send = frame.toString();
            connectionHandler.sendLine(send);
        }
    }
}

void StompProtocol::handleReport(vector<string> read){
    // Parse the events file specified in read[1]
    names_and_events information;
    try {
        information = parseEventsFile(read[1]);
        string channel = information.channel_name;
        std::vector<Event> events = information.events;
        for (Event& event : events) {
            // Create a frame for each event 
            event.setEventOwnerUser(connectionHandler.get_user_name());
            Frame frame("SEND", {{"destination", channel}, {"user", event.getEventOwnerUser()} , {"city", event.get_city()}
            , {"event name", event.get_name()}, {"date time", std::to_string(event.get_date_time())}
            ,{"general information", event.get_general_information()}
            ,{"description", event.get_description()}}, "");
            // Send the frame
        }

    } catch (const std::exception& e) {
        cerr << "Error parsing events file: " << e.what() << endl;
    }
}

void StompProtocol::handleSummery(vector<string> read){
    // To do 

}
