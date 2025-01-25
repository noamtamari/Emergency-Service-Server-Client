#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/Frame.h"
#include "../include/Event.h"
#include <set>
#include <mutex>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler *connectionHandler;
    int receipts = 0;
    int subscription_id = 0;
    int logout_reciept = -1;
    std::mutex lock_connection;
    bool connected = false;
    unordered_map<string, int> channel_subscription = {}; // From channel to subscription id
    unordered_map<int, string> unsubscribe_channel = {}; // From receipt of exit to subscription id
    unordered_map<int, string> receipt_respons = {}; // From user's action's receipt to relevent output to the user when the server succseed to perform the user's action
    unordered_map<int, int> receipt_subscriptionId = {}; // From receipt of subscription, to the subscriptionId of the user 
    unordered_map<int, string> receipt_channels = {}; // From receipt of subscription, to the channel subscribe to
    unordered_map<int, string> receipt_map = {}; // From receipt of command, to the command 
    unordered_map<string, unordered_map<string, vector<Event>>> summary = {};
    std::set<string> receipt_validator;

public:
    StompProtocol(ConnectionHandler *connectionHandler);
    virtual ~StompProtocol();
    bool isConnected();
    void setConnected(bool status);
    bool processServerFrame(const string &frame);
    void processUserInput(vector<string> read);
    void handleLogin(vector<string> read);
    void handleJoin(vector<string> read);
    void handleExit(vector<string> read);
    void handleReport(vector<string> read);
    void handleLogout(vector<string> read);
    void handleSummary(vector<string> read);
    void handleError(Frame frame);
    void handleMessage(Frame frame);
    void handleConnected(Frame frame);
    void handleReciept(Frame frame);
    Frame parseFrame(const std::string &frame);
    // Frame parseFrame(const string& input);
    static bool eventComparator(const Event& e1, const Event& e2);
    const string summerize_description(const string &string);
    const string epoch_to_date(const string &date_and_time);
    void exportEventsToFile(const string &channel,const string &user,const string &filename);
    void printSummary(const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>>& summary);
};
