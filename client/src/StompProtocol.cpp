#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include "../include/json.hpp"
#include "StompProtocol.h"
#include "../include/keyboardInput.h"
#include "../include/Event.h" // Ensure this contains the definitions for Frame, Event, and names_and_events

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream> // For file operations
#include <vector>

using namespace std;

// Initializes the protocol with a given `ConnectionHandler`.
StompProtocol::StompProtocol(ConnectionHandler *connectionHandler) : connectionHandler(connectionHandler) {}

StompProtocol::~StompProtocol() {}

/**
 * @brief Checks whether the connection to the server is active.
 *
 * Uses a mutex to ensure thread safety.
 *
 * @return true if the connection is active, false otherwise.
 */
bool StompProtocol::isConnected()
{
    std::lock_guard<std::mutex> lock(lock_connection);
    return connected;
}

/**
 * @brief Sets the connection status.
 *
 * Uses a mutex to safely update the `connected` flag.
 *
 * @param status Boolean indicating whether the connection is active.
 */
void StompProtocol::setConnected(bool status)
{
    std::lock_guard<std::mutex> lock(lock_connection);
    connected = status;
}

// -----------------------------Server Frames----------------------------------------

/**
 * @brief Processes a server frame.
 *
 * Delegates handling to specific functions based on the frame's command.
 *
 * @param frame The raw frame string received from the server.
 * @return true if the frame was successfully processed, false otherwise.
 */
bool StompProtocol::processServerFrame(const std::string &frame)
{
    Frame newFrame = parseFrame(frame); // Parse the raw frame into a structured `Frame` object

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
    return false; // Unknown frame command
}

/**
 * @brief Handles error frames sent by the server.
 *
 * Displays the error message and disconnects the client.
 *
 * @param frame The error frame sent by the server.
 */
void StompProtocol::handleError(Frame frame)
{
    const string message = frame.getHeader("The message");
    std::cout << "\033[31mERROR FROM THE SERVER: \n \033[0m" << endl;
    std::cout << "\033[31m" + frame.getBody() + "\033[0m" << endl;
    setConnected(false); // Disconnect the client
}

/**
 * @brief Handles message frames sent by the server.
 *
 * Adds events to a summary map grouped by user and channel.
 *
 * @param frame The message frame sent by the server.
 */
void StompProtocol::handleMessage(Frame frame)
{
    string report_frame = frame.getBody();                 // Extract the body of the message
    const string channel = frame.getHeader("destination"); // Extract the channel name
    Event event(report_frame, channel);                    // Create an `Event` object from the frame body

    unordered_map<string, unordered_map<string, vector<Event>>>::iterator user_reported = summary.find(event.getEventOwnerUser()); // Map of the user previous reports for all channels

    // User does not have previous reports
    if (user_reported == summary.end())
    {
        unordered_map<string, vector<Event>> report_map = {};
        vector<Event> reports_for_channel_vector;
        reports_for_channel_vector.push_back(event);
        report_map.emplace(event.get_channel_name(), reports_for_channel_vector);
        summary.emplace(event.getEventOwnerUser(), report_map);
    }
    else // User already has reports
    {
        unordered_map<string, vector<Event>> &previous_user_reports = user_reported->second;
        std::unordered_map<string, vector<Event>>::iterator event_channel = previous_user_reports.find(event.get_channel_name());

        if (event_channel == previous_user_reports.end()) // New channel for this user
        {
            vector<Event> reports_for_channel_vector;
            reports_for_channel_vector.push_back(event);
            previous_user_reports.emplace(event.get_channel_name(), reports_for_channel_vector);
        }

        else // Existing channel for this user
        {
            vector<Event> &reports_vector = event_channel->second; 
            reports_vector.push_back(event);
        }
    }
}

/**
 * @brief Handles connected frames sent by the server.
 * 
 * Initializes the user's report map and marks the client as connected.
 * 
 * @param frame The connected frame received from the server.
 */
void StompProtocol::handleConnected(Frame frame)
{
    unordered_map<string, vector<Event>> report_map = {}; // Create an empty report map for the user
    summary.emplace(connectionHandler->get_user_name(), report_map); // Associate it with the client's user name
}

/**
 * @brief Processes receipt frames and performs actions based on receipt IDs.
 * 
 * Manages join, exit, logout, and report commands based on the received receipt ID.
 * 
 * @param frame The receipt frame sent by the server.
 */
void StompProtocol::handleReciept(Frame frame)
{
    const string receipt = frame.getHeader("receipt-id");
    std::unordered_map<int, std::string>::iterator output = receipt_respons.find(std::stoi(receipt));  // Locate the response for this receipt
    if (output == receipt_respons.end())
    {
        std::cout << "receipt got lost" << std::endl;
    }
    else
    {
        unordered_map<int, std::string>::iterator action_receipt = receipt_map.find(std::stoi(receipt));
        bool print_now = true;

        if (action_receipt != receipt_map.end())  // Check for specific actions tied to this receipt
        {
            const string command = action_receipt->second;
            if (command == "join") { // Handle channel join
                unordered_map<int, int>::iterator isSubscription = receipt_subscriptionId.find(std::stoi(receipt));

                if (isSubscription != receipt_subscriptionId.end()) // Handle successful subscription
                {
                    unordered_map<int, string>::iterator receipt_channel = receipt_channels.find(std::stoi(receipt));
                    const string subscription_channel = receipt_channel->second;
                    unordered_map<string, int>::iterator not_subscribed = channel_subscription.find(subscription_channel);
                    
                    if (not_subscribed == channel_subscription.end()) // Add to channel subscriptions
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

            if (command == "exit") {  // Handle channel exit
                std::unordered_map<int, std::string>::iterator channel = unsubscribe_channel.find(std::stoi(receipt));
                if (channel != unsubscribe_channel.end())
                {
                    channel_subscription.erase(channel->second);
                }
                receipt_map.erase(std::stoi(receipt));
            }

            if (command == "logout") { // Handle logout
                if (std::to_string(logout_reciept) == receipt)
                {
                    setConnected(false);
                }
                receipt_map.erase(std::stoi(receipt));
            }

            if (command == "report") { // Handle report
                print_now = false;
                unordered_map<int, int>::iterator receipt_counter = receipt_counter_map.find(std::stoi(receipt));
                if (receipt_counter != receipt_counter_map.end()){
                    int &reciept_count = receipt_counter->second;
                    if (reciept_count == 2){
                        receipt_map.erase(std::stoi(receipt));
                        receipt_counter_map.erase(std::stoi(receipt));
                    }
                    else{
                        reciept_count = reciept_count - 1;
                    }
                }
            }
        }
        if (print_now) // Print the response if needed
        {
            std::cout << "\033[32m" + output->second + "\033[0m" << std::endl;
            receipt_respons.erase(std::stoi(receipt));
        }
    }
}

// -----------------------------Client Frames----------------------------------------

/**
 * @brief Processes user input commands.
 * 
 * Delegates commands to their respective handlers (join, exit, report, logout, summary).
 * 
 * @param read A vector of strings representing the parsed user input.
 */
void StompProtocol::processUserInput(vector<string> read)
{
    if (read.size() != 0){
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
            std::cout << "\033[95mIllegal command, please try a different one\033[0m" << std::endl;
        }
    }
    else{
        std::cout << "\033[95mIllegal command, please try a different one\033[0m" << std::endl;
    }
}

/**
 * @brief Handles the login command by sending a CONNECT frame to the server.
 * 
 * @param read A vector of strings representing the parsed user input.
 */
void StompProtocol::handleLogin(vector<string> read){
    receipts++;
    receipt_respons.emplace(receipts, "Login successful"); // Store the receipt response

    Frame frame("CONNECT", {{"accept-version", "1.2"},
                {"host", "stomp.cs.bgu.ac.il"},
                {"receipt", std::to_string(receipts)},
                {"login", read[2]}, {"passcode", read[3]}}, ""); // Construct the CONNECT frame
    string send = frame.toString();

    (*connectionHandler).sendLine(send);  // Send the frame
}

/**
 * @brief Handles the logout command by sending a DISCONNECT frame to the server.
 * 
 * @param read A vector of strings representing the parsed user input.
 */
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
        receipt_respons.emplace(receipts, "Logged Out"); // Store the receipt response
        logout_reciept = receipts;

        Frame frame("DISCONNECT", {{"receipt", std::to_string(receipts)}}, ""); // Construct DISCONNECT frame
        string send = frame.toString();

        (*connectionHandler).sendLine(send); // Send the frame
    }
}

/**
 * @brief Handles the `join` command, subscribing the user to a channel.
 * 
 * @param read A vector of strings representing the parsed user input.
 */
void StompProtocol::handleJoin(vector<string> read)
{
    if (read.size() != 2)
    {
        std::cout << "\033[95mjoin command needs 1 args: {channel_name}\033[0m" << std::endl;
    }
    else
    {
        int join_subscription_id = subscription_id + 1; // Increment subscription ID
        receipts++;
        receipt_map.emplace(receipts, "join");
        receipt_subscriptionId.emplace(receipts, join_subscription_id);
        receipt_channels.emplace(receipts, read[1]);
        receipt_respons.emplace(receipts, "Joined channel " + read[1]);

        Frame frame("SUBSCRIBE", {{"destination", read[1]}, {"receipt", std::to_string(receipts)}, {"id", std::to_string(join_subscription_id)}}, ""); // Construct SUBSCRIBE frame
        string send = frame.toString();

        (*connectionHandler).sendLine(send); // Send the frame
    }
}

/**
 * @brief Handles the `exit` command, unsubscribing the user from a channel.
 * 
 * @param read A vector of strings representing the parsed user input.
 */
void StompProtocol::handleExit(vector<string> read)
{
    if (read.size() != 2)
    {
        std::cout << "\033[95mexit command needs 1 args: {channel}\033[0m" << std::endl;
    }
    else
    {
        std::unordered_map<std::string, int>::iterator channel_subscriptionId = channel_subscription.find(read[1]);

        // Check if the user is subscribed to the specified channel
        if (channel_subscriptionId == channel_subscription.end())
        {
            std::cout << "\033[95myou are not subscribed to channel\033[0m" << std::endl;
        }
        else
        {
            receipts++;
            receipt_map.emplace(receipts, "exit");  // Mark this receipt as an `exit` action
            receipt_respons.emplace(receipts, "Exited channel " + channel_subscriptionId->first);
            unsubscribe_channel.emplace(receipts, channel_subscriptionId->first);

            Frame frame("UNSUBSCRIBE", {{"destination", read[1]}, {"receipt", to_string(receipts)}, {"id", to_string(channel_subscriptionId->second)}}, "");  // Construct UNSUBSCRIBE frame
            string send = frame.toString();

            (*connectionHandler).sendLine(send);  // Send the frame to the server
        }
    }
}

/**
 * @brief Handles the `report` command by parsing a file of events and sending them to the server.
 * 
 * @param read A vector of strings representing the parsed user input.
 */
void StompProtocol::handleReport(vector<string> read)
{
    if (read.size() != 2)
    {
        std::cout << "\033[95mreport command needs 1 args: {file}\033[0m" << std::endl;
    }
    else
    {
        try
        {
            names_and_events information = parseEventsFile(read[1]);  // Parse the events file
            string channel = information.channel_name;
            vector<Event> &events = information.events;
            receipts++;
            int reciepts_count = 0;
            receipt_respons.emplace(receipts, "reported");

            // Loop through each event and send it to the server
            for (Event &event : events)
            {
                reciepts_count++;
                event.setEventOwnerUser((*connectionHandler).get_user_name());

                Frame frame("SEND", {{"destination", channel}, {"receipt", to_string(receipts)}}, event.toString()); // Construct SEND frame
                string send = frame.toString();

                (*connectionHandler).sendLine(send); // Send the frame to the server
            }

            // If there are multiple events, store the receipt count for proper tracking
            if (reciepts_count > 1){
                receipt_map.emplace(receipts, "report");
                receipt_counter_map.emplace(receipts, reciepts_count);
            }
        }
        catch (const std::exception &e)
        {
            cerr << "Error parsing events file: " << e.what() << endl;
        }
    }
}

/**
 * @brief Handles the `summary` command by exporting events for a specific user and channel to a file.
 * 
 * @param read A vector of strings representing the parsed user input.
 */
void StompProtocol::handleSummary(vector<string> read)
{
    if (read.size() != 4)
    {
        std::cout << "\033[95summary command needs 3 args: {channel_name} {user} {file}\033[0m" << std::endl;
    }
    else
    {
        // Locate the user's summary in the report map
        unordered_map<string, unordered_map<string, vector<Event>>>::iterator channel_reported = summary.find(connectionHandler->get_user_name());
        unordered_map<string, vector<Event>>::iterator was_subscribed = (channel_reported->second).find(read[1]);

        // Check if the user is subscribed to the given channel
        if (was_subscribed == (channel_reported->second).end()){
            std::cout << "you are not subscribed to channel " + read[1] << endl;
        }
        else{
            string file_path = "../bin/" + read[3]; // Construct the file path for the output
            exportEventsToFile(read[1], read[2], file_path); // Export events to file
        }
    }
}

//--------------------------General functions----------------------------------------

/**
 * @brief Parses a raw STOMP frame string into a structured Frame object.
 * 
 * This function splits the raw frame string into its components: type, headers, and body.
 * 
 * @param input The raw STOMP frame string.
 * @return Frame A structured Frame object representing the parsed input.
 */
Frame StompProtocol::parseFrame(const string &input)
{
    istringstream stream(input);
    string line;

    getline(stream, line);
    string type = line;

    // Parse headers
    unordered_map<string, string> headers;
    while (std::getline(stream, line, '\n') && !line.empty())
    {
        if (line != "\n")
        {
            auto colonPos = line.find(':');
            string key = line.substr(0, colonPos);
            string value = line.substr(colonPos + 1);
            headers[key] = value;
        }
        else
        {
            break;  // Stop at an empty line (end of headers)
        }
    }

    // Parse the body (everything after the blank line)
    std::string body;
    if (std::getline(stream, line, '\0'))
    {
        body = line;
    }
    return Frame(type, headers, body);  // Return the parsed Frame object
} 


/**
 * @brief Summarizes a description by truncating it to 27 characters, appending "..." if necessary.
 * 
 * @param description The original description string.
 * @return string The summarized description.
 */
const string StompProtocol::summerize_description(const string &description)
{
    if (description.length() > 27){
        return description.substr(0, 27) + "..."; // Truncate to 27 chars and add "..."
    }
    else{
        return description; // Return the original description if it's short enough
    }
}

/**
 * @brief Converts an epoch timestamp string to a human-readable date and time format.
 * 
 * @param input The epoch timestamp string.
 * @return string The formatted date and time string.
 */
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

/**
 * @brief Exports event reports for a specific user and channel to a JSON file.
 * 
 * @param channel The name of the channel to export events from.
 * @param user The name of the user whose events are being exported.
 * @param filename The name of the file to export the event data to.
 * 
 * @details 
 * This function checks if the specified user and channel exist in the summary data. 
 * If they do, it calculates statistics such as the number of active events, total events, 
 * and events involving "forces arrival at scene." It then writes the event details and 
 * statistics to the specified file. If the user or channel does not exist, it exports an 
 * empty report file with default statistics.
 */
void StompProtocol::exportEventsToFile(const string &channel, const string &user, const string &filename)
{
    // Ensure the user exists in the summary
    unordered_map<string, unordered_map<string, vector<Event>>>::iterator user_reported = summary.find(user);
    int true_active = 0; // Number of events marked as active
    int total_sum_reports = 0; // Total number of events
    int forces = 0; // Number of events involving "forces arrival at scene"

    // If the user is not found in the summary, export an empty file and return
    if (user_reported == summary.end())
    {
        exportEmptyFile(channel, filename);
        return;
    }
    // Check if the specified channel exists in the user's reports
    unordered_map<string, vector<Event>> &previous_user_reports = user_reported->second;
    unordered_map<string, vector<Event>>::iterator event_channel = previous_user_reports.find(channel);

    // If the channel is not found, export an empty file and return
    if (event_channel == previous_user_reports.end())
    {
        exportEmptyFile(channel, filename);
        return;
    }


    vector<Event> &reports_vector = event_channel->second;
    // Sort the events in the channel by time, and then by name if times are equal
    std::sort(reports_vector.begin(), reports_vector.end(), eventComparator);

    // Loop through each event in the vector and calculate statistics
    for (const Event &event : reports_vector)
    {
        // Retrieve the general information map for the event
        map<string, string> general_info = event.get_general_information();
        const string active = " active";

        // Check if the event is marked as "active" and increment the counter
        if ((event.get_general_information()).find(active)->second == "true")
        {
            true_active++;
        }
        const string force = " forces_arrival_at_scene";
        
        // Check if the event involved "forces arrival at scene" and increment the counter
        if ((event.get_general_information()).find(force)->second == "true")
        {
            forces++;
        }

        // Increment the total number of events
        total_sum_reports++;
    }

    // Create an output file stream to write the data
    std::ofstream output_file;
    output_file.open(filename);

    // If the file could not be opened, display an error message
    if (!output_file)
    {
        cerr << "Error: could not open the file" << endl;
    }
    else
    {
        // Write the header and statistics to the output file
        output_file << "Channel " + channel << endl;
        output_file << "Stats: " << endl;
        output_file << "Total: " << std::to_string(total_sum_reports) << endl;
        output_file << "active: " << std::to_string(true_active) << endl;
        output_file << "forces arrival at scene: " << std::to_string(forces) << endl;
        output_file << "" << endl;
        output_file << "Event Resports:" << endl;
        output_file << "" << endl;

        // Write details of each event in the channel
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

/**
 * @brief Exports an empty event report file with default statistics.
 * 
 * @param channel The name of the channel for the empty report.
 * @param filename The name of the file to export the empty report to.
 * 
 * @details 
 * This function creates a file with no event data and default statistics (all set to 0). 
 * It is used when the user or channel does not exist in the summary data.
 */
void StompProtocol::exportEmptyFile(const string &channel, const string &filename)
{
    // Create an output file stream
    std::ofstream output_file;
    output_file.open(filename);


    // If the file could not be opened, display an error message
    if (!output_file)
    {
        cerr << "Error: could not open the file" << endl;
    }
    else
    {
        // Write the default header and empty statistics
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

/**
 * @brief Compares two events for sorting based on date and name.
 * 
 * @param e1 The first event to compare.
 * @param e2 The second event to compare.
 * @return True if `e1` should come before `e2` in sorted order; false otherwise.
 * 
 * @details 
 * This comparator first compares events by their date (timestamp). 
 * If the timestamps are equal, it compares the events by their names lexicographically.
 */
bool StompProtocol::eventComparator(const Event &e1, const Event &e2)
{
    // If the events have the same time, sort by name
    if (e1.get_date_time() == e2.get_date_time())
    {
        return e1.get_name() < e2.get_name(); // Compare names lexicographically
    }
    return e1.get_date_time() < e2.get_date_time(); // Otherwise, sort by time
}