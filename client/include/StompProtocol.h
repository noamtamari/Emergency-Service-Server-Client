#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/Frame.h"
#include <set>


// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler *connectionHandler;
    int reciepts = 0;
    int subscription_id = 0;
    int logout_reciept = -1;
    bool connected = false;
    unordered_map<string, int> channel_subscription = {};
    unordered_map<int, string> unsubscribe_channel = {};
    unordered_map<int, string> reciept_respons = {};
    unordered_map<string, unordered_map<string, vector<Event>>> summery = {};
    std::set<string> receipt_validator;

public:
    StompProtocol(ConnectionHandler *connectionHandler);
    void processServerFrame(const std::string &frame);
    void processUserInput(vector<string> read);
    void handleLogin(vector<string> read);
    void handleJoin(vector<string> read);
    void handleExit(vector<string> read);
    void handleReport(vector<string> read);
    void handleLogout(vector<string> read);
    void handleSummery(vector<string> read);
    void handleError(Frame frame);
    void handleMessage(Frame frame);
    void handleConnected(Frame frame);
    void handleReciept(Frame frame);

    void addReceipt(int receipt);
    bool hasReceipt(int receipt) const;
    void removeReceipt(int receipt);

};
