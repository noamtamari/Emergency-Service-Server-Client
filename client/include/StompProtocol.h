#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/Frame.h"
#include "../include/Event.h"

#include <mutex>

class StompProtocol
{
private:
    ConnectionHandler *connectionHandler;
    int receipts = 0;
    int subscription_id = 0;
    int logout_reciept = -1;
    std::mutex lock_connection= {};
    bool connected = false; // Tracks whether the client is connected to the server.

    unordered_map<string, int> channel_subscription = {}; // Maps channel names to their subscription IDs.

    unordered_map<int, string> unsubscribe_channel = {}; // Maps receipt to channels for unsubscribing.
    unordered_map<int, string> receipt_respons = {}; // Maps receipt to output for user responses.
    unordered_map<int, int> receipt_subscriptionId = {}; // Maps receipt IDs to subscription IDs.
    unordered_map<int, string> receipt_channels = {}; // Maps receipt of subscription to subscription IDs.
    unordered_map<int, string> receipt_map = {}; // Maps receipt IDs to the corresponding command name.
    unordered_map<int, int> receipt_counter_map= {}; // Maps receipt IDs to counters for managing multi-step commands. Uses for multipule reports

    unordered_map<string, unordered_map<string, vector<Event>>> summary = {}; // Stores summaries of events grouped by users and channels.

public:
    StompProtocol(ConnectionHandler *connectionHandler); // Constructor initializing the protocol with the given connection handler.
    virtual ~StompProtocol();
    
    StompProtocol(const StompProtocol &other) = delete; // Disables copying via the copy constructor.
    StompProtocol& operator=(const StompProtocol &other) = delete; // Disables copying via the assignment operator.

    bool isConnected();
    void setConnected(bool status);

    void processUserInput(vector<string> read);

    // User input protocols
    void handleLogin(vector<string> read);
    void handleJoin(vector<string> read);
    void handleExit(vector<string> read);
    void handleReport(vector<string> read);
    void handleLogout(vector<string> read);
    void handleSummary(vector<string> read);

    bool processServerFrame(const string &frame);

    // Server protocols
    void handleError(Frame frame); 
    void handleMessage(Frame frame); 
    void handleConnected(Frame frame);
    void handleReciept(Frame frame);

    
    Frame parseFrame(const string &frame); // Parses a raw STOMP frame string into a structured `Frame` object.
    static bool eventComparator(const Event& e1, const Event& e2); // Comparator function to sort events by time and name.
    const string summerize_description(const string &string); // Summarizes an event description to 27 characters, appending "..." if necessary.
    const string epoch_to_date(const string &date_and_time); // Converts an epoch timestamp to a human-readable date and time format.
    void exportEventsToFile(const string &channel,const string &user,const string &filename); // Exports event data for a specific channel and user to a file.
    void exportEmptyFile (const string &channel, const string &filename); // Writes an empty file if no events are available for the given channel and user.
};
