#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include <string>
#include <iostream>
#include "../include/keyboardInput.h"
#include <vector>
#include <regex>
#include <unordered_map>
#include "../include/Event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events
#include "StompProtocol.h"
#include <map>
#include "../include/json.hpp"
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>

using namespace std;

// Constructor for StompProtocol
// Initializes the protocol with the given ConnectionHandler
StompProtocol::StompProtocol(ConnectionHandler *connectionHandler) : connectionHandler(connectionHandler) {}

bool StompProtocol::isConnected()
{
    return connected;
}

// Processes a server frame by delegating it to specific function based on the frame command
bool StompProtocol::processServerFrame(const std::string &frame)
{
    Frame newFrame = parseFrame(frame);
    // cout << "Frame to proccess: " << newFrame.toString() << endl;
    // cout << "Command to proccess: " << newFrame.getCommand() << endl;
    const string command = newFrame.getCommand();
    if (command == "RECEIPT")
    {
        handleReciept(newFrame);
        return true;
    }
    if (newFrame.getCommand() == "CONNECTED")
    {
        handleConnected(newFrame);
        return true;
    }
    if (newFrame.getCommand() == "MESSAGE")
    {
        handleMessage(newFrame);
        return true;
    }
    if (newFrame.getCommand() == "ERROR")
    {
        handleError(newFrame);
        return true;
    }
}


// Handles error frames from the server
void StompProtocol::handleError(Frame frame)
{
    // check somehow what failed, mabey we do need reveipt-id althoug i think we send it
    cout << "ERROR FROM THE SERVER: \n \n"
         << endl;
}

// Handles message frames by adding events to the summary map, grouped by user and channel
void StompProtocol::handleMessage(Frame frame)
{
    string report_frame = frame.getBody();
    Event event(report_frame);
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>>::iterator user_reported = summary.find(event.getEventOwnerUser()); // Map of the user previous reports for all channels
    // User did not report previously for any channel
    if (user_reported == summary.end())
    {
        unordered_map<string, vector<Event>> report_map = {};
        vector<Event> reports_for_channel_vector;
        reports_for_channel_vector.push_back(event);
        report_map.emplace(event.get_channel_name(), reports_for_channel_vector);
        summary.emplace(event.getEventOwnerUser(), report_map);
    }
    // User already reported previously
    else
    {
        unordered_map<string, vector<Event>> previous_user_reports = user_reported->second;
        std::unordered_map<string, vector<Event>>::iterator event_channel = previous_user_reports.find(event.get_channel_name());
        // User reported for a new channel
        if (event_channel == previous_user_reports.end())
        {
            vector<Event> reports_for_channel_vector;
            reports_for_channel_vector.push_back(event);
            previous_user_reports.emplace(event.get_channel_name(), reports_for_channel_vector);
        }
        // User already reported for event's channel
        else
        {
            vector<Event> reports_vector = event_channel->second;
            reports_vector.push_back(event);
        }
    }
}

// Handles connected frames, initializes the user's report map, and marks the client as connected
void StompProtocol::handleConnected(Frame frame)
{
    unordered_map<string, vector<Event>> report_map = {};
    summary.emplace(connectionHandler->get_user_name(), report_map);
    connected = true;
}

// Processes receipt frames and performs actions based on receipt IDs
void StompProtocol::handleReciept(Frame frame)
{
    const string receipt = frame.getHeader("receipt-id");
    std::unordered_map<int, std::string>::iterator output = receipt_respons.find(std::stoi(receipt));
    if (output == receipt_respons.end())
    {
        cout << "receipt got lost" << endl;
    }
    // Print output of receipt 
    else
    {
        // Print current output
        std::cout << "\033[95m" +  output->second + "\033[0m" << std::endl;
        cout << "Reciept from server: " << receipt << endl;
        receipt_respons.erase(std::stoi(receipt));
        //cout << output->second << endl;
        unordered_map<int, std::string>::iterator action_receipt = receipt_map.find(std::stoi(receipt));
        // Requierd further handling for the relevent receipt in the client : exit,join,logout
        if (action_receipt != receipt_map.end()){
            const string command = action_receipt->second;
            if (command == "join"){
                 unordered_map<int, int>::iterator isSubscription = receipt_subscriptionId.find(std::stoi(receipt));
                // Receipt of subscription 
                if(isSubscription != receipt_subscriptionId.end()){
                    unordered_map<int, string>::iterator receipt_channel = receipt_channels.find(std::stoi(receipt));
                    const string subscription_channel = receipt_channel->second;
                    unordered_map<string, int>::iterator is_already_subscribed = channel_subscription.find(subscription_channel);
                    // Client was alreeady subscribed
                    if (is_already_subscribed == channel_subscription.end())
                        channel_subscription.emplace(receipt_channel->second, isSubscription->second);
                    receipt_subscriptionId.erase(std::stoi(receipt));
                    receipt_channels.erase(std::stoi(receipt));
                }
            }
            // Reciept of exit 
            if (command == "exit"){
                std::unordered_map<int, std::string>::iterator channel = unsubscribe_channel.find(std::stoi(receipt));
                if (channel != unsubscribe_channel.end())
                {
                    channel_subscription.erase(channel->second);
                }
            }
            if (command == "logout"){
                    if (std::to_string(logout_reciept) == receipt)
                    {
                        // logout protocol
                    }
            }
            receipt_map.erase(std::stoi(receipt));
        }
    }

}

// Processes user input and delegates to specific handlers based on the command
void StompProtocol::processUserInput(vector<string> read)
{
    if (read[0] == "join")
    {
        handleJoin(read);
    }
    else if (read[0] == "exit")
    {
        handleExit(read);
    }
    else if (read[0] == "report")
    {
        handleReport(read);
    }
    else if (read[0] == "logout")
    {
        handleLogout(read);
    }
    else if (read[0] == "summary")
    {
        handleSummary(read);
    }
    else
    {
        cout << "Illegal command, please try a different one" << endl;
    }
}

// Handles the login command by sending a CONNECT frame to the server
void StompProtocol::handleLogin(vector<string> read)
{
    std::cout << "login succesful" << std::endl;
    receipts++;
    receipt_respons.emplace(receipts, "login succesful");
    Frame frame("CONNECT", {{"accept-version", "1.2"}, {"receipt", std::to_string(receipts)}, {"login", read[2]}, {"passcode", read[3]}}, "");
    string send = frame.toString();
    (*connectionHandler).sendLine(send);
}

// Handles the logout command by sending a DISCONNECT frame to the server
void StompProtocol::handleLogout(vector<string> read)
{
    if (read.size() != 1)
    {
        std::cout << "" << std::endl;
    }
    else
    {
        // connected = false;
        receipts++;
        receipt_map.emplace(receipts,"logout");
        receipt_respons.emplace(receipts, "Logged Out");
        logout_reciept = receipts;
        Frame frame("DISCONNECT", {{"receipt", std::to_string(receipts)}}, "");
        string send = frame.toString();
        (*connectionHandler).sendLine(send);
    }
}

// Handles the join command, subscribing the user to a channel
void StompProtocol::handleJoin(vector<string> read)
{
    if (read.size() != 2)
    {
        cout << "" << endl;
    }
    else
    {
        int join_subscription_id = subscription_id + 1;
         // Adding the channel to my user summery reports
        std::unordered_map<string, unordered_map<string, vector<Event>>>::iterator my_user = summary.find(connectionHandler->get_user_name());
        std::unordered_map<string, vector<Event>> previous_user_reports = my_user->second;
        std::unordered_map<string, vector<Event>>::iterator event_channel = previous_user_reports.find(read[1]);
        if (event_channel == previous_user_reports.end())
        {
            cout << read[1] << endl;
            vector<Event> reports_for_channel_vector;
            previous_user_reports.emplace(read[1], reports_for_channel_vector);    
        }
        receipts++;
        receipt_map.emplace(receipts,"join");
        receipt_subscriptionId.emplace(receipts,join_subscription_id);
        receipt_channels.emplace(receipts,read[1]);
        receipt_respons.emplace(receipts, "Joined channel " + read[1]);
        Frame frame("SUBSCRIBE", {{"destination", read[1]}, {"receipt", std::to_string(receipts)}, {"id", std::to_string(join_subscription_id)}}, "");
        string send = frame.toString();
        (*connectionHandler).sendLine(send);
    }
}

// Handles the `exit` command by unsubscribing the user from a channel
void StompProtocol::handleExit(vector<string> read)
{
    if (read.size() != 2)
    {
        std::cout << "" << std::endl;
    }
    else
    {
        std::unordered_map<std::string, int>::iterator channel_subscriptionId = channel_subscription.find(read[1]);
        // User is not subscribed to the specified channel
        if (channel_subscriptionId == channel_subscription.end())
        {
            cout << "you are not subscribed to channel " + channel_subscriptionId->first << endl;
        }
        else
        {
            // Unsubscribe from the channel
            receipts++;
            receipt_map.emplace(receipts,"exit");
            receipt_respons.emplace(receipts, "Exited channel " + channel_subscriptionId->first);
            unsubscribe_channel.emplace(receipts, channel_subscriptionId->first);
            Frame frame("UNSUBSCRIBE", {{"destination", read[1]}, {"receipt", to_string(receipts)}, {"id", to_string(channel_subscriptionId->second)}}, "");
            string send = frame.toString();
            (*connectionHandler).sendLine(send);
        }
    }
}

// Handles the `report` command by parsing a file of events and sending them to the server
void StompProtocol::handleReport(vector<string> read)
{
    if (read.size() != 2)
    {
        cout << "report command needs 1 args: {file}" << endl;
    }
    else
    {
        // Parse the events file specified in read[1]
        try
        {
            std::unordered_map<string, unordered_map<string, vector<Event>>>::iterator my_user = summary.find(connectionHandler->get_user_name());
            std::unordered_map<string, vector<Event>> previous_user_reports = my_user->second;
            std::unordered_map<string, vector<Event>>::iterator event_channel = previous_user_reports.find("police");
            if (event_channel ==  previous_user_reports.end()){
                cout << "CHANRKEORLW E" << endl;
            }
            names_and_events information = parseEventsFile(read[1]); // Extract events from the file
            string channel = information.channel_name;
            std::vector<Event> events = information.events;
            // Create a frame for each event and send it to the server
            for (Event &event : events)
            {
                receipts++;
                receipt_respons.emplace(receipts, "reported");
                event.setEventOwnerUser((*connectionHandler).get_user_name());
                Frame frame("SEND", {{"destination", channel}, {"receipt", to_string(receipts)}}, event.toString());
                string send = frame.toString();
                (*connectionHandler).sendLine(send);
            }
        }
        catch (const std::exception &e)
        {
            cerr << "Error parsing events file: " << e.what() << endl;
        }
    }
}

// Handles the `summary` command by exporting events from a specific channel and user to a file
void StompProtocol::handleSummary(vector<string> read)
{
    if (read.size() != 4)
    {
        std::cout << "summary command needs 3 args: {channel_name} {user} {file}" << std::endl;
    }
    else
    {
        // Check if the channel exists in the summary
        // std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>>::iterator channel_reported = summary.find(read[1]);
        // if (channel_reported == summary.end())
        // {
        //     cout << "you are not subscribed to channel " + read[1] << endl;
        // }
        // else
        // {
            exportEventsToFile(read[1], read[2], "../bin/" + read[3]);
        // }
    }
}

// Parses a raw STOMP frame string into a structured Frame object
Frame StompProtocol::parseFrame(const string &input)
{
    istringstream stream(input);
    string line;

    // Parse the frame type (first line)
    if (!getline(stream, line) || line.empty())
    {
        //throw std::invalid_argument("Invalid frame: Missing type.");
    }
    string type = line;

    // Parse headers
    unordered_map<string, string> headers;
    while (std::getline(stream, line) && !line.empty())
    {
        auto colonPos = line.find(':');
        // if (colonPos == std::string::npos)
        // {
        //     throw std::invalid_argument("Invalid frame: Malformed header.");
        // }
        string key = line.substr(0, colonPos);
        string value = line.substr(colonPos + 1);
        headers[key] = value;
    }
    // Parse body (if present)
    string body;
    if (std::getline(stream, line, '\0'))
    { // Read until null terminator
        body = line;
    }
    return Frame(type, headers, body);
}

// Summarizes a description by truncating it to 27 characters, appending "..." if necessary
const string StompProtocol::summerize_description(const string &description)
{
    if (description.length() > 27)
    {
        return description.substr(0, 27) + "..."; // Truncate to 27 chars and add "..."
    }
    else
    {
        return description; // Return the original string if it's not longer than 27 chars
    }
}

// Converts an epoch timestamp string to a human-readable date and time format
const string StompProtocol::epoch_to_date(const string &date_and_time)
{
    return date_and_time.substr(0, 2) + "/" + date_and_time.substr(2, 4) + "/" + date_and_time.substr(4, 6) + " " + date_and_time.substr(6, 8) + ":" + date_and_time.substr(8, 10);
}

// Exports event reports for a specific user and channel to a JSON file
void StompProtocol::exportEventsToFile(const string &channel, const string &user, const string &filename)
{
    // Ensure the user exists in the summary
    unordered_map<string, unordered_map<string, vector<Event>>>::iterator user_iter = summary.find(user);
    bool empty;
    if (user_iter == summary.end())
    {
        cerr << "Error: User not found in summary." << endl;
        empty = true;
    }
    // Ensure the channel exists in the user's reports
    unordered_map<string, vector<Event>> user_reports = user_iter->second;
    unordered_map<string, vector<Event>>::iterator channel_iter = user_reports.find(channel);
    if (channel_iter == user_reports.end())
    {
        cerr << "Error: Channel not found in user's reports." << endl;
        return;
    }
    // Analyze general information
    int true_active = 0;
    int total_sum_reports = 0;
    int forces = 0;
    vector<Event> report_from_channel = channel_iter->second;
    map<int, map<string, Event>> sorted_events;
    if (!empty)
    {
        for (const Event &event : report_from_channel)
        {
            map<int, map<string, Event>>::iterator same_date_events = sorted_events.find(event.get_date_time());
            if (same_date_events == sorted_events.end())
            {
                map<string, Event> event_name_and_event = {};
                event_name_and_event.insert({event.get_name(), event});
            }
            else
            {
                map<string, Event> event_name_and_event = same_date_events->second;
                event_name_and_event.insert({event.get_name(), event});
            }
            map<string, string> general_info = event.get_general_information();
            if ((general_info.find("active")->second) == "true")
            {
                true_active++;
            }
            if ((general_info.find("forces arrival at scene")->second) == "true")
            {
                forces++;
            }
            total_sum_reports++;
        }
    }
    std::ofstream output_file;
    output_file.open(filename);
    if (!output_file)
    {
        cerr << "Error: could not open the file" << endl;
    }
    else
    {
        output_file << "Channel " + channel << endl;
        output_file << "Stats: " << endl;
        output_file << "Total: " + total_sum_reports << endl;
        output_file << "active: " + true_active << endl;
        output_file << "forces arrival at scene: " + forces << endl;
        output_file << "" + channel << endl;
        output_file << "Event Resports:" << endl;
        output_file << "Channel " + channel << endl;
        if (!empty)
        {
            int reports = 1;
            // Iterate using an iterator
            for (map<int, map<string, Event>>::iterator it = sorted_events.begin(); it != sorted_events.end(); ++it)
            {
                map<string, Event> event_name_to_event = it->second;
                for (map<string, Event>::iterator iter = event_name_to_event.begin(); iter != event_name_to_event.end(); ++iter)
                {
                    Event event = iter->second;
                    output_file << "Report_" + to_string(reports) + channel << endl;
                    output_file << "\tcity: " + event.get_city() << endl;
                    output_file << "\tdate time: " + epoch_to_date(to_string(event.get_date_time())) << endl;
                    output_file << "\tevent name: " + event.get_name() << endl;
                    output_file << "\tsummary: " + summerize_description(event.get_description()) << endl;
                    output_file << "" + channel << endl;
                }
            }
        }
    }
}
