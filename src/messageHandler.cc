#include "messageHandler.h"
#include <iostream>

MessageHandler::MessageHandler(Connection& connection, bool debug)
    : conn(connection), debug(debug)
{
}

void MessageHandler::printInt(const std::string& code, int value) {
    if (debug) {
        std::cout << code << " " << value << std::endl;
    }
}

void MessageHandler::printString(const std::string& code, const std::string& value) {
    if (debug) {
        std::cout << code << " " << value << std::endl;
    }
}

void MessageHandler::sendByte(int code) {
    try {
        conn.write(static_cast<unsigned char>(code));
        printInt("[sendByte]", code);
    } catch (ConnectionClosedException&) {
        throw;
    }
}

int MessageHandler::recvByte() {
    try {
        int code = conn.read();
        printInt("[recvByte]", code);
        return code;
    } catch (ConnectionClosedException&) {
        throw;
    }
}

void MessageHandler::sendCode(Protocol code) {
    sendByte(static_cast<int>(code));
    printInt("[sendCode]", static_cast<int>(code));
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
    printString("[sendStringParam]", param);
}

int MessageHandler::recvCode() {
    int code = recvByte();
    printInt("[recvCode]", code);
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
    printString("[recvStringParameter]", result);
    return result;
}
