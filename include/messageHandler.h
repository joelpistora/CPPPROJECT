#pragma once
#include "connection.h"
#include "connectionclosedexception.h"
#include "protocol.h"
#include <iostream>
#include <string>
#include <stdexcept>

class ProtocolViolationException : public std::runtime_error {
public:
    ProtocolViolationException(const std::string& msg) 
        : std::runtime_error(msg) {}
};

class MessageHandler {
private:
    Connection& conn;
    bool debug;

    void sendByte(int code);
    int recvByte();
    
    void printInt(const std::string& code, int value);
    void printString(const std::string& code,  const std::string& value);
public:
    MessageHandler(Connection& connection, bool debug = true);


    void sendCode(Protocol code);
    void sendInt(int value);
    void sendIntParameter(int param);
    void sendStringParameter(const std::string& param);
    
    int recvCode();
    int recvInt();
    int recvIntParameter();
    std::string recvStringParameter();
};
