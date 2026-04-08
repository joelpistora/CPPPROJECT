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

    void sendStringParameter(const std::string& param) {
        sendCode(static_cast<int>(Protocol::PAR_STRING));
        sendInt(static_cast<int>(param.size()));
        for (char c : param) {
            sendByte(static_cast<unsigned char>(c));
        }
        std::cout << "[sendStringParam] " << param << std::endl;
    }

    int recvCode() {
        int code = recvByte();
        std::cout << "[recvCode] " << code << std::endl;
        return code;
    }

    int recvInt() {
        int b1 = recvByte();
        std::cout << "[recvInt] " << b1 << std::endl;
        int b2 = recvByte();
        std::cout << "[recvInt] " << b2 << std::endl;
        int b3 = recvByte();
        std::cout << "[recvInt] " << b3 << std::endl;
        int b4 = recvByte();
        std::cout << "[recvInt] " << b4 << std::endl;
    }

    int recvIntParameter() {
        int code = recvCode();
        if (code != static_cast<int>(Protocol::PAR_NUM)) {
            throw ProtocolViolationException("Receive numeric parameter");
        }
        std::cout << "[recvIntParameter] " << code << std::endl;
        return recvInt();
    }

    std::string recvStringParameter() {
        int code = recvCode();
        if (code != static_cast<int>(Protocol::PAR_STRING)) {
            throw ProtocolViolationException("Receive string parameter");
        }
        int n = recvInt();
        if (n < 0) {
            throw ProtocolViolationException("Receive string parameter");
        }
        std::string result;
        result.reserve(n);
        for (int i = 0; i < n; ++i) {
            char ch = static_cast<char>(recvByte());
            result + ch;
        }
        std::cout << "[recvStringParameter] " << result << std::endl;
        return result;
    }
};