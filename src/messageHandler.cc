#include "messageHandler.h"
#include <iostream>

MessageHandler::MessageHandler(Connection& connection)
    : conn(connection)
{
}

void MessageHandler::sendByte(int code) {
    try {
        conn.write(static_cast<unsigned char>(code));
        std::cout << "[sendByte] " << code << std::endl;
    } catch (ConnectionClosedException&) {
        throw;
    }
}

int MessageHandler::recvByte() {
    try {
        int code = conn.read();
        std::cout << "[recvByte] " << code << std::endl;
        return code;
    } catch (ConnectionClosedException&) {
        throw;
    }
}

void MessageHandler::sendCode(Protocol code) {
    sendByte(static_cast<int>(code));
    std::cout << "[sendCode] " << static_cast<int>(code) << std::endl;
}

void MessageHandler::sendInt(int value) {
    sendByte((value >> 24) & 0xFF);
    sendByte((value >> 16) & 0xFF);
    sendByte((value >> 8) & 0xFF);
    sendByte(value & 0xFF);
}

void MessageHandler::sendIntParameter(int param) {
    sendCode(Protocol::PAR_NUM);
    sendInt(param);
}

void MessageHandler::sendStringParameter(const std::string& param) {
    sendCode(Protocol::PAR_STRING);
    sendInt(static_cast<int>(param.size()));
    for (char c : param) {
        sendByte(static_cast<unsigned char>(c));
    }
    std::cout << "[sendStringParam] " << param << std::endl;
}

int MessageHandler::recvCode() {
    int code = recvByte();
    std::cout << "[recvCode] " << code << std::endl;
    return code;
}

int MessageHandler::recvInt() {
    unsigned char b1 = static_cast<unsigned char>(recvByte());
    unsigned char b2 = static_cast<unsigned char>(recvByte());
    unsigned char b3 = static_cast<unsigned char>(recvByte());
    unsigned char b4 = static_cast<unsigned char>(recvByte());
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}

int MessageHandler::recvIntParameter() {
    int code = recvCode();
    if (code != static_cast<int>(Protocol::PAR_NUM)) {
        throw ProtocolViolationException("Receive numeric parameter");
    }
    return recvInt();
}

std::string MessageHandler::recvStringParameter() {
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
        result += ch;
    }
    std::cout << "[recvStringParameter] " << result << std::endl;
    return result;
}
