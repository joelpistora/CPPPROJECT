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

    void sendByte(int code);
    int recvByte();

public:
    MessageHandler(Connection& connection);

    void sendCode(Protocol code);
    void sendInt(int value);
    void sendIntParameter(int param);
    void sendStringParameter(const std::string& param);
    
    int recvCode();
    int recvInt();
    int recvIntParameter();
    std::string recvStringParameter();
};
