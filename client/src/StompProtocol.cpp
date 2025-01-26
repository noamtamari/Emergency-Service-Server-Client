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

StompProtocol::~StompProtocol(){}

bool StompProtocol::isConnected()
{
    std::lock_guard<std::mutex> lock(lock_connection);
    return connected;
}

void StompProtocol::setConnected(bool status)
{
    std::lock_guard<std::mutex> lock(lock_connection);
    connected = status;
}

// Processes a server frame by delegating it to specific function based on the frame command
bool StompProtocol::processServerFrame(const std::string &frame)
{
    Frame newFrame = parseFrame(frame);
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
    return false;
}

// Handles error frames from the server
void StompProtocol::handleError(Frame frame)
{
    // check somehow what failed, mabey we do nee
    const string message = frame.getHeader("The message");
    std::cout << "\033[31mERROR FROM THE SERVER: \n \033[0m" << endl;
    std::cout << "\033[31m" + frame.getBody() + "\033[0m" << endl;
    setConnected(false);
}

// Handles message frames by adding events to the summary map, grouped by user and channel
void StompProtocol::handleMessage(Frame frame)
{
    string report_frame = frame.getBody();
    const string receipt = frame.getHeader("receipt-id");
    const string channel = frame.getHeader("destination");
    Event event(report_frame, channel);
    unordered_map<string, unordered_map<string, vector<Event>>>::iterator user_reported = summary.find(event.getEventOwnerUser()); // Map of the user previous reports for all channels
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
        unordered_map<string, vector<Event>> &previous_user_reports = user_reported->second;
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
            vector<Event> &reports_vector = event_channel->second; // Use a reference here
            reports_vector.push_back(event);
        }
    }
}

// Handles connected frames, initializes the user's report map, and marks the client as connected
void StompProtocol::handleConnected(Frame frame)
{
    unordered_map<string, vector<Event>> report_map = {};
    summary.emplace(connectionHandler->get_user_name(), report_map);
}

// Processes receipt frames and performs actions based on receipt IDs
void StompProtocol::handleReciept(Frame frame)
{
    const string receipt = frame.getHeader("receipt-id");
    std::unordered_map<int, std::string>::iterator output = receipt_respons.find(std::stoi(receipt));
    if (output == receipt_respons.end())
    {
        std::cout << "receipt got lost" << std::endl;
    }
    // Print output of receipt
    else
    {
        unordered_map<int, std::string>::iterator action_receipt = receipt_map.find(std::stoi(receipt));
        bool print_now = true;
        // Requierd further handling for the relevent receipt in the client : exit,join,logout
        if (action_receipt != receipt_map.end())
        {
            const string command = action_receipt->second;
            if (command == "join")
            {
                unordered_map<int, int>::iterator isSubscription = receipt_subscriptionId.find(std::stoi(receipt));
                // Receipt of subscription
                if (isSubscription != receipt_subscriptionId.end())
                {
                    unordered_map<int, string>::iterator receipt_channel = receipt_channels.find(std::stoi(receipt));
                    const string subscription_channel = receipt_channel->second;
                    unordered_map<string, int>::iterator not_subscribed = channel_subscription.find(subscription_channel);
                    // Client was alreeady subscribed
                    if (not_subscribed == channel_subscription.end())
                    {
                        channel_subscription.emplace(receipt_channel->second, isSubscription->second);
                        std::unordered_map<string, unordered_map<string, vector<Event>>>::iterator my_user = summary.find(connectionHandler->get_user_name());
                        vector<Event> reports_for_channel_vector;
                        (my_user->second).emplace(subscription_channel, reports_for_channel_vector);
                    }
                    receipt_subscriptionId.erase(std::stoi(receipt));
                    receipt_channels.erase(std::stoi(receipt));
                }
                receipt_map.erase(std::stoi(receipt));
            }
            // Reciept of exit
            if (command == "exit")
            {
                std::unordered_map<int, std::string>::iterator channel = unsubscribe_channel.find(std::stoi(receipt));
                if (channel != unsubscribe_channel.end())
                {
                    channel_subscription.erase(channel->second);
                }
                receipt_map.erase(std::stoi(receipt));
            }
            if (command == "logout")
            {
                if (std::to_string(logout_reciept) == receipt)
                {
                    setConnected(false);
                }
                receipt_map.erase(std::stoi(receipt));
            }
            if (command == "report"){
                print_now = false;
                unordered_map<int,int>::iterator receipt_counter = receipt_counter_map.find(std::stoi(receipt));
                if (receipt_counter != receipt_counter_map.end()){
                    int &reciept_count = receipt_counter->second;
                    if (reciept_count == 2){
                        receipt_map.erase(std::stoi(receipt));
                        receipt_counter_map.erase(std::stoi(receipt));
                    }
                    else{
                        reciept_count = reciept_count-1;
                    }
                }

            }
        }
        if (print_now){
            // Print current output
            std::cout << "\033[32m"+  output->second +"\033[0m" << std::endl;
            receipt_respons.erase(std::stoi(receipt));
        }
    }
}

// Processes user input and delegates to specific handlers based on the command
void StompProtocol::processUserInput(vector<string> read)
{
    if (read.size() != 0){
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
            std::cout << "\033[95mIllegal command, please try a different one\033[0m" << std::endl;
        }
    }
    else{
        std::cout << "\033[95mIllegal command, please try a different one\033[0m" << std::endl;
    }
}

// Handles the login command by sending a CONNECT frame to the server
void StompProtocol::handleLogin(vector<string> read)
{
    receipts++;
    receipt_respons.emplace(receipts, "Login successful");
    Frame frame("CONNECT", {{"accept-version", "1.2"}, {"host", "stomp.cs.bgu.ac.il"},{"receipt", std::to_string(receipts)}, {"login", read[2]}, {"passcode", read[3]}}, "");
    string send = frame.toString();
    (*connectionHandler).sendLine(send);
}

// Handles the logout command by sending a DISCONNECT frame to the server
void StompProtocol::handleLogout(vector<string> read)
{
    if (read.size() != 1)
    {
        std::cout << "\033[95mlogout command needs 0 arg\033[0m" << std::endl;
    }
    else
    {
        receipts++;
        receipt_map.emplace(receipts, "logout");
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
        std::cout << "\033[95mjoin command needs 1 args: {channel_name}\033[0m" << std::endl;
    }
    else
    {
        int join_subscription_id = subscription_id + 1;
        // Adding the channel to my user summery reports
        receipts++;
        receipt_map.emplace(receipts, "join");
        receipt_subscriptionId.emplace(receipts, join_subscription_id);
        receipt_channels.emplace(receipts, read[1]);
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
        std::cout << "\033[95mexit command needs 1 args: {channel}\033[0m" << std::endl;
    }
    else
    {
        std::unordered_map<std::string, int>::iterator channel_subscriptionId = channel_subscription.find(read[1]);
        channel_subscription.end();

        // User is not subscribed to the specified channel
        if (channel_subscriptionId == channel_subscription.end())
        {
            std::cout << "\033[95myou are not subscribed to channel\033[0m" << std::endl;
        }
        else
        {
            // Unsubscribe from the channel
            receipts++;
            receipt_map.emplace(receipts, "exit");
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
        std::cout << "\033[95mreport command needs 1 args: {file}\033[0m" << std::endl;
    }
    else
    {
        // Parse the events file specified in read[1]
        try
        {
            names_and_events information = parseEventsFile(read[1]); // Extract events from the file
            string channel = information.channel_name;
            vector<Event> &events = information.events;
            receipts++;
            int reciepts_count = 0;
            receipt_respons.emplace(receipts, "reported");
            // Create a frame for each event and send it to the server
            for (Event &event : events)
            {
                reciepts_count++;
                event.setEventOwnerUser((*connectionHandler).get_user_name());
                Frame frame("SEND", {{"destination", channel}, {"receipt", to_string(receipts)}}, event.toString());
                string send = frame.toString();
                (*connectionHandler).sendLine(send);
            }
            if (reciepts_count > 1){
                receipt_map.emplace(receipts,"report");
                receipt_counter_map.emplace(receipts,reciepts_count);
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
        std::cout << "\033[95summary command needs 3 args: {channel_name} {user} {file}\033[0m" << std::endl;
    }
    else
    {
        // Check if the channel exists in the summary
        unordered_map<string, unordered_map<string, vector<Event>>>::iterator channel_reported = summary.find(connectionHandler->get_user_name());
        unordered_map<string, vector<Event>>::iterator was_subscribed = (channel_reported->second).find(read[1]);
        if (was_subscribed == (channel_reported->second).end())
        {
            std::cout << "you are not subscribed to channel " + read[1] << endl;
        }
        // else
        {
            string file_path = "../bin/" + read[3];
            exportEventsToFile(read[1], read[2], file_path);
        }
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
        // throw std::invalid_argument("Invalid frame: Missing type.");
    }
    string type = line;

    // Parse headers
    unordered_map<string, string> headers;
    while (std::getline(stream, line, '\n') && !line.empty())
    {
        if (line != "\n")
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
        else
        {
            break;
        }
    }
    // Parse body (if present)
    // Parse the body (everything after the blank line)
    std::string body;
    if (std::getline(stream, line, '\0'))
    {
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
const string StompProtocol::epoch_to_date(const string &input)
{
    // Convert epoch seconds to time_t
    long epochSeconds = std::stol(input);
    std::time_t time = static_cast<std::time_t>(epochSeconds);

    // Convert time_t to tm structure for local time
    std::tm *localTime = std::localtime(&time);

    // Use a stringstream to format the date and time
    std::ostringstream oss;
    oss << std::put_time(localTime, "%d/%m/%Y %H:%M");
    return oss.str();
}

// Exports event reports for a specific user and channel to a JSON file
void StompProtocol::exportEventsToFile(const string &channel, const string &user, const string &filename)
{
    // Ensure the user exists in the summary
    unordered_map<string, unordered_map<string, vector<Event>>>::iterator user_reported = summary.find(user);
    int true_active = 0;
    int total_sum_reports = 0;
    int forces = 0;
    if (user_reported == summary.end())
    {
        exportEmptyFile(channel,filename);
        return;
    }
    // Ensure the channel exists in the user's reports
    unordered_map<string, vector<Event>> &previous_user_reports = user_reported->second;
    unordered_map<string, vector<Event>>::iterator event_channel = previous_user_reports.find(channel);
    if (event_channel == previous_user_reports.end())
    {
        exportEmptyFile(channel,filename);
        return;
    }
    // Analyze general information
    vector<Event> &reports_vector = event_channel->second;
    std::sort(reports_vector.begin(), reports_vector.end(), eventComparator);
    map<int, map<string, Event>> sorted_events = {};
    for (const Event &event : reports_vector)
    {
        map<string, string> general_info = event.get_general_information();
        const string active = " active";
        if ((event.get_general_information()).find(active)->second == "true"){
            true_active++;
        }
        const string force = " forces_arrival_at_scene";
        if ((event.get_general_information()).find(force)->second == "true"){
            forces++;
        }
        total_sum_reports++;
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
        output_file << "Total: " << std::to_string(total_sum_reports) << endl;
        output_file << "active: " << std::to_string(true_active) << endl;
        output_file << "forces arrival at scene: " << std::to_string(forces) << endl;
        output_file << "" << endl;
        output_file << "Event Resports:" << endl;
        output_file << "" << endl;
        int reports = 1;
        for (const Event &event : reports_vector)
        {
            output_file << "Report_" + to_string(reports) + ":" << endl;
            output_file << "\tcity: " + event.get_city() << endl;
            output_file << "\tdate time: " + epoch_to_date(to_string(event.get_date_time())) << endl;
            output_file << "\tevent name: " + event.get_name() << endl;
            output_file << "\tsummary: " + summerize_description(event.get_description()) << endl;
            output_file << "" << endl;
            reports++;
        }
    }
}

void StompProtocol::exportEmptyFile (const string &channel, const string &filename){
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
        output_file << "Total: " << std::to_string(0) << endl;
        output_file << "active: " << std::to_string(0) << endl;
        output_file << "forces arrival at scene: " << std::to_string(0) << endl;
        output_file << "" << endl;
        output_file << "Event Resports:" << endl;
        output_file << "" << endl;
    }

}

bool StompProtocol::eventComparator(const Event &e1, const Event &e2)
{
    if (e1.get_date_time() == e2.get_date_time())
    {
        return e1.get_name() < e2.get_name(); // Sort by name if time is equal
    }
    return e1.get_date_time() < e2.get_date_time(); // Otherwise, sort by time
}