#pragma once
#include "include/connection.h"
#include "include/connectionclosedexception.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <include/protocol.h>

class ProtocolViolationException : public std::runtime_error {
public:
    ProtocolViolationException(const std::string& msg) 
        : std::runtime_error(msg) {}
};

class MessageHandler {
private:
    Connection& conn;

    void sendByte(int code) {
        try {
            conn.write(static_cast<unsigned char>(code));
            std::cout << "[sendByte] " << code << std::endl;
        } catch (ConnectionClosedException&) {
            throw;
        }
    }

    int recvByte() {
        try {
            int code = conn.read();
            std::cout << "[recvByte] " << code << std::endl;
            return code;
        } catch (ConnectionClosedException&) {
            throw;
        }
    }

public:
    MessageHandler(Connection& connection) : conn(connection) {}

    void sendCode(int code) {
        sendByte(code);
        std::cout << "[sendCode] " << code << std::endl;
    }

    void sendInt(int value) {
        sendByte((value >> 24) & 0xFF);
        std::cout << "[sendByte] " << ((value >> 24) & 0xFF) << std::endl;
        sendByte((value >> 16) & 0xFF);
        std::cout << "[sendByte] " << ((value >> 16) & 0xFF) << std::endl;
        sendByte((value >> 8) & 0xFF);
        std::cout << "[sendByte] " << ((value >> 8) & 0xFF) << std::endl;
        sendByte(value & 0xFF);
        std::cout << "[sendByte] " << (value & 0xFF) << std::endl;
    }

    void sendIntParameter(int param) {
        sendCode(static_cast<int>(Protocol::PAR_NUM));
        sendInt(param);
    }
}