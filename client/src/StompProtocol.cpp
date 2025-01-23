#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include <string>
#include <iostream>
#include "../include/keyboardInput.h"
#include <vector>
#include <regex>
#include <unordered_map>
#include "../include/Event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler *connectionHandler) :
connectionHandler(connectionHandler) {}

void StompProtocol::processServerFrame(const std::string &frame){
    Frame newFrame = parseFrame(frame);
    if (newFrame.getCommand() == "RECIEPT"){
        handleReciept(newFrame);
    }
    if (newFrame.getCommand() == "CONNECTED"){
        handleConnected(newFrame);
    }
    if (newFrame.getCommand() == "MESSAGE"){
        handleMessage(newFrame);
    }
    if (newFrame.getCommand() == "ERROR"){
        handleError(newFrame);
    }
}

void StompProtocol::handleError(Frame frame){
    // TODO: write this function 
}

void StompProtocol::handleMessage(Frame frame){
    string report_frame = frame.getBody();
    Event event(report_frame);
    auto user_reported = summery.find(event.getEventOwnerUser()); // Map of the user previous reports for all channels 
    // User did not report previously for any channel
    if (user_reported == summery.end()){
        unordered_map<string, vector<Event>> report_map = {};
        vector<Event> reports_for_channel_vector;
        reports_for_channel_vector.push_back(event);
        report_map.emplace(event.get_channel_name(),reports_for_channel_vector);
        summery.emplace(event.getEventOwnerUser(),report_map);
    }
    // User already reported previously
    else{
        unordered_map<string, vector<Event>> previous_user_reports = user_reported->second;
        auto event_channel = previous_user_reports.find(event.get_channel_name());
        // User reported for a new channel
        if (event_channel == previous_user_reports.end()){
            vector<Event> reports_for_channel_vector;
            reports_for_channel_vector.push_back(event);
            previous_user_reports.emplace(event.get_channel_name(),reports_for_channel_vector);
        }
        // User already reported for event's channel
        else{
            vector<Event> reports_vector = event_channel->second;
            reports_vector.push_back(event);
        }
    }
}

void StompProtocol::handleConnected(Frame frame){
    connected = true;
}

void StompProtocol::handleReciept(Frame frame){
    string reciept = frame.getHeader("reciept");
    auto output = reciept_respons.find(std::stoi(reciept));
    if (output == reciept_respons.end()){
        cout << "recipet got lost" << endl;
    }
    else{
        // Print current output  
       cout << output->second << endl;
       reciept_respons.erase(std::stoi(reciept));
    }
    if (std::to_string(logout_reciept) == reciept){
       // logout protocol
    }
    auto channel = unsubscribe_channel.find(std::stoi(reciept));
    if(channel != unsubscribe_channel.end()){
        channel_subscription.erase(channel->second);
    }
}

void StompProtocol::processUserInput(vector<string> read){
    if (read[0] == "login"){
        handleJoin(read);
    }
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

void StompProtocol::handleLogin(vector<string> read){
    //std::cout << "login succesful" << std::endl;
    reciept_respons.emplace(reciepts++, "login succesful");
    Frame frame("CONNECT", {{"accept-version", "1.2"},{"receipt", std::to_string(reciepts)}, {"login", read[2]}, {"passcode", read[3]}}, "");
    string send = frame.toString();
    (*connectionHandler).sendLine(send);
}

void StompProtocol::handleLogout(vector<string> read){
    if (read.size() != 1){
        std::cout << "" << std::endl;
    }
    else{
        //connected = false;
        reciept_respons.emplace(reciepts++, "Logged Out");
        logout_reciept = reciepts;
        Frame frame("DISCONNECT", {{"receipt", std::to_string(reciepts)}},"");
        string send = frame.toString();
        (*connectionHandler).sendLine(send);
    }
}

void StompProtocol::handleJoin(vector<string> read){
    if (read.size() != 2){
        cout << "" << endl;
    }
    else{
        auto channel_subscribed = channel_subscription.find(read[1]);
        if (channel_subscribed != channel_subscription.end()){
            cout << "Joined channel " + read[1];
        }
        else{
             int join_subscription_id = subscription_id+1;
            channel_subscription.emplace(read[1], join_subscription_id);
            reciept_respons.emplace(reciepts++,  "Joined channel " + read[1]);
            Frame frame("SUBSCRIBE", {{"destination", read[1]},{"receipt", std::to_string(reciepts)}, {"id", std::to_string(join_subscription_id)}},"");
            string send = frame.toString();
            (*connectionHandler).sendLine(send);
        }
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
            //cout << "Exited channel " + channel_subscriptionId->first << endl;
            reciept_respons.emplace(reciepts++, "Exited channel " + channel_subscriptionId->first);
            unsubscribe_channel.emplace(reciepts,channel_subscriptionId->first);
            Frame frame("UNSUBSCRIBE", {{"destination", read[1]},{"receipt", to_string(reciepts)}, {"id", to_string(channel_subscriptionId->second)}},"");
            string send = frame.toString();
            (*connectionHandler).sendLine(send);
        }
    }
}

void StompProtocol::handleReport(vector<string> read){
    if (read.size() != 2){
        cout << "report command needs 1 args: {file}" << endl;
    }
    else{
        // Parse the events file specified in read[1]
        names_and_events information;
        try {
            information = parseEventsFile(read[1]);
            string channel = information.channel_name;
            std::vector<Event> events = information.events;
            for (Event& event : events) {
                // Create a frame for each event 
                reciept_respons.emplace(reciepts++, "reported");
                event.setEventOwnerUser((*connectionHandler).get_user_name());
                Frame frame("SEND", {{"destination", channel},{"receipt", to_string(reciepts)}}, event.toString());
                string send = frame.toString();
                (*connectionHandler).sendLine(send);
            }
        } catch (const std::exception& e) {
            cerr << "Error parsing events file: " << e.what() << endl;
        }
    }
}

void StompProtocol::handleSummery(vector<string> read){
    if (read.size() != 4){
        std::cout << "summary command needs 3 args: {channel_name} {user} {file}" << std::endl;
    }
    else{
        // put something of summery
        // TODO: write this function
        reciept_respons.emplace(reciepts++, "reported");
    }

}

// To be check 
Frame StompProtocol::parseFrame(const string& input) {
    istringstream stream(input);
    string line;

    // Parse the frame type (first line)
    if (!getline(stream, line) || line.empty()) {
        throw std::invalid_argument("Invalid frame: Missing type.");
    }
    string type = line;

    // Parse headers
    unordered_map<string, string> headers;
    while (std::getline(stream, line) && !line.empty()) {
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            throw std::invalid_argument("Invalid frame: Malformed header.");
        }
        string key = line.substr(0, colonPos);
        string value = line.substr(colonPos + 1);
        headers[key] = value;
    }
    // Parse body (if present)
    string body;
    if (std::getline(stream, line, '\0')) { // Read until null terminator
        body = line;
    }
    return Frame(type, headers, body);
}



