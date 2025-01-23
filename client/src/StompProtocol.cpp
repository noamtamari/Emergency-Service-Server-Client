#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include <string>
#include <iostream>
#include "../include/keyboardInput.h"
#include <vector>
#include <regex>
#include <unordered_map>
#include "../include/event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events
#include "StompProtocol.h"
#include <map>
#include "../include/json.hpp"
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>

using namespace std;
using json = nlohmann::json;

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
    // check somehow what failed, mabey we do need reveipt-id althoug i think we send it 
    cout << "ERROR FROM THE SERVER: \n \n" << endl;
}

void StompProtocol::handleMessage(Frame frame){
    string report_frame = frame.getBody();
    Event event(report_frame);
    auto user_reported = summary.find(event.getEventOwnerUser()); // Map of the user previous reports for all channels 
    // User did not report previously for any channel
    if (user_reported == summary.end()){
        unordered_map<string, vector<Event>> report_map = {};
        vector<Event> reports_for_channel_vector;
        reports_for_channel_vector.push_back(event);
        report_map.emplace(event.get_channel_name(),reports_for_channel_vector);
        summary.emplace(event.getEventOwnerUser(),report_map);
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
    unordered_map<string, vector<Event>> report_map = {};
    summary.emplace(connectionHandler->get_user_name(), report_map);
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
    else if (read[0] == "summary"){
        handleSummary(read);
    }
    else{
        cout << "Illegal command, please try a different one" << endl;
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
            // Adding the channel to my user summery reports 
            auto my_user = summary.find(connectionHandler->get_user_name());
            unordered_map<string, vector<Event>> previous_user_reports = my_user->second;
            auto event_channel = previous_user_reports.find(read[1]);
            if (event_channel == previous_user_reports.end()){
                vector<Event> reports_for_channel_vector;
                previous_user_reports.emplace(read[1],reports_for_channel_vector);
            }
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

void StompProtocol::handleSummary(vector<string> read){
    if (read.size() != 4){
        std::cout << "summary command needs 3 args: {channel_name} {user} {file}" << std::endl;
    }
    else{
        auto channel_reported = summary.find(read[1]);
        if (channel_reported == summary.end()){
            cout << "you are not subscribed to channel " + read[1] << endl;
        }
        else{
           exportEventsToJSON(read[1],read[2],read[3]); 
        }
    }

}

// To be check 
Frame parseFrame(const string& input) {
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


const string StompProtocol::summerize_description(const string &description){
    if (description.length() > 27) {
        return description.substr(0, 27) + "..."; // Truncate to 27 chars and add "..."
    } else {
        return description; // Return the original string if it's not longer than 27 chars
    }
}

const string StompProtocol::epoch_to_date(const string &date_and_time){
    return date_and_time.substr(0,2) + "/" +  date_and_time.substr(2,4) + "/" + date_and_time.substr(4,6) + " " + date_and_time.substr(6,8) + ":"+ date_and_time.substr(8,10);
}

void StompProtocol::exportEventsToJSON(const string& channel,const string& user, const string& filename) {
    // Ensure the user exists in the summary
    auto user_iter = summary.find(user);
    if (user_iter == summary.end()) {
        std::cerr << "Error: User not found in summary." << std::endl;
        return;
    }
    // Ensure the channel exists in the user's reports
    const auto& user_reports = user_iter->second;
    auto channel_iter = user_reports.find(channel);
    if (channel_iter == user_reports.end()) {
        std::cerr << "Error: Channel not found in user's reports." << std::endl;
        return;
    }
    // Extract and sort events by date_time
    const auto& report_from_channel = channel_iter->second;
    std::map<int, Event> sorted_events;
    for (const Event& event : report_from_channel) {
        sorted_events.insert({event.get_date_time(), event});
    }

    json output = json::array();
}



