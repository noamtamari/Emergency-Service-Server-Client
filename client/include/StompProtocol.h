#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/StompEncoderDecoder.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
private: 
    ConnectionHandler &connectionHandler;
    StompEncoderDecoder encoderDecoder;
    int reciepts = 0;
    int subscription_id = 0;
    bool connected = false;
    unordered_map<string,int> channel_subscription = {};

public: 
    StompProtocol(ConnectionHandler &connectionHandler, StompEncoderDecoder encoderDecoder);
    void handleInput(vector<string> read);
    void handleJoin(vector<string> read);
    void handleExit(vector<string> read);
    void handleReport(vector<string> read);
    void handleLogout(vector<string> read);
    void handleSummery(vector<string> read);


};
