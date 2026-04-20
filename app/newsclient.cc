#include "connection.h"
#include "connectionclosedexception.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <messageHandler.h>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

// Parse next token from an istringstream. Supports quoted tokens with double quotes.
static bool readToken(std::istringstream& iss, std::string& out) {
    iss >> std::ws;
    int c = iss.peek();
    if (c == '"') {
        iss.get(); // consume '"'
        std::getline(iss, out, '"');
        return true;
    }
    return static_cast<bool>(iss >> out);
}

// Perform newsgroup creation using MessageHandler. Returns false if connection lost.
static bool doCreateNG(MessageHandler& mh, const std::string& name) {
    try {
        mh.sendCode(Protocol::COM_CREATE_NG);
        mh.sendStringParameter(name);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_CREATE_NG)) {
            throw ProtocolViolationException("Expected ANS_CREATE_NG");
        }

        int status = mh.recvCode();
        if (status == static_cast<int>(Protocol::ANS_ACK)) {
            cout << "Success: Newsgroup '" << name << "' created." << endl;
        } else {
            int err = mh.recvCode();
            if (err == static_cast<int>(Protocol::ERR_NG_ALREADY_EXISTS)) {
                cout << "Error: Newsgroup name '" << name << "' is already taken." << endl;
            } else {
                cout << "Error: Operation failed with code " << err << endl;
            }
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Error: Connection lost while creating newsgroup." << endl;
        return false;
    }
}

// Perform listing of newsgroups. Returns false if connection lost.
static bool doListNewsgroups(MessageHandler& mh) {
    try {
        mh.sendCode(Protocol::COM_LIST_NG);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_LIST_NG)) {
            throw ProtocolViolationException("Expected ANS_LIST_NG");
        }

        int numGroups = mh.recvIntParameter();
        cout << "Number of newsgroups: " << numGroups << endl;

        for (int i = 0; i < numGroups; ++i) {
            int id = mh.recvIntParameter();
            string name = mh.recvStringParameter();
            cout << " " << id << ". " << name << endl;
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Server disconnected." << endl;
        return false;
    }
}

// Perform newsgroup deletion using MessageHandler. Returns false if connection lost.
static bool doDeleteNG(MessageHandler& mh, int ngId) {
    try {
        mh.sendCode(Protocol::COM_DELETE_NG);
        mh.sendIntParameter(ngId);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_DELETE_NG)) {
            throw ProtocolViolationException("Expected ANS_DELETE_NG");
        }

        int status = mh.recvCode();
        if (status == static_cast<int>(Protocol::ANS_ACK)) {
            cout << "Success: Newsgroup with id " << ngId << " deleted." << endl;
        } else {
            int err = mh.recvCode();
            if (err == static_cast<int>(Protocol::ERR_NG_DOES_NOT_EXIST)) {
                cout << "Error: Newsgroup with id " << ngId << " does not exist." << endl;
            } else {
                cout << "Error: Operation failed with code " << err << endl;
            }
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Error: Connection lost while creating newsgroup." << endl;
        return false;
    }
}

// Perform listing of articles in a newsgroup. Returns false if connection lost.
static bool doListArticles(MessageHandler& mh, int ngId) {
    try {
        mh.sendCode(Protocol::COM_LIST_ART);
        mh.sendIntParameter(ngId);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_LIST_ART)) {
            throw ProtocolViolationException("Expected ANS_LIST_ART");
        }

        int status = mh.recvCode();
        if (status == static_cast<int>(Protocol::ANS_ACK)) {
            int numArticles = mh.recvIntParameter();
            cout << "Number of articles: " << numArticles << endl;

            for (int i = 0; i < numArticles; ++i) {
                int id = mh.recvIntParameter();
                string title = mh.recvStringParameter();
                cout << " " << id << ". " << title << endl;
            }
        } else {
            int err = mh.recvCode();
            if (err == static_cast<int>(Protocol::ERR_NG_DOES_NOT_EXIST)) {
                cout << "Error: Newsgroup " << ngId << " does not exist." << endl;
            } else {
                cout << "Error: Operation failed with code " << err << endl;
            }
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;

        int numGroups = mh.recvIntParameter();
        cout << "Number of newsgroups: " << numGroups << endl;

        for (int i = 0; i < numGroups; ++i) {
            int id = mh.recvIntParameter();
            string name = mh.recvStringParameter();
            cout << " " << id << ". " << name << endl;
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Server disconnected." << endl;
        return false;
    }
}

// Perform article creation using MessageHandler. Returns false if connection lost.
static bool doCreateArticle(MessageHandler& mh, int ngId, const std::string& title,
                            const std::string& author, const std::string& text) {
    try {
        mh.sendCode(Protocol::COM_CREATE_ART);
        mh.sendIntParameter(ngId);
        mh.sendStringParameter(title);
        mh.sendStringParameter(author);
        mh.sendStringParameter(text);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_CREATE_ART)) {
            throw ProtocolViolationException("Expected ANS_CREATE_ART");
        }

        int status = mh.recvCode();
        if (status == static_cast<int>(Protocol::ANS_ACK)) {
            cout << "Success: Article created in newsgroup " << ngId << "." << endl;
        } else {
            int err = mh.recvCode();
            if (err == static_cast<int>(Protocol::ERR_NG_DOES_NOT_EXIST)) {
                cout << "Error: Newsgroup " << ngId << " does not exist." << endl;
            } else {
                cout << "Error: Operation failed with code " << err << endl;
            }
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Error: Connection lost while creating article." << endl;
        return false;
    }
}

// Perform article deletion using MessageHandler. Returns false if connection lost.
static bool doDeleteArticle(MessageHandler& mh, int ngId, int aId) {
    try {
        mh.sendCode(Protocol::COM_DELETE_ART);
        mh.sendIntParameter(ngId);
        mh.sendIntParameter(aId);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_DELETE_ART)) {
            throw ProtocolViolationException("Expected ANS_DELETE_ART");
        }

        int status = mh.recvCode();
        if (status == static_cast<int>(Protocol::ANS_ACK)) {
            cout << "Success: Article with id " << aId << " deleted." << endl;
        } else {
            int err = mh.recvCode();
            if (err == static_cast<int>(Protocol::ERR_NG_DOES_NOT_EXIST)) {
                cout << "Error: Newsgroup " << ngId << " does not exist." << endl;
            } else if (err == static_cast<int>(Protocol::ERR_ART_DOES_NOT_EXIST)) {
                cout << "Error: Article " << aId << " does not exist." << endl;
            } else {
                cout << "Error: Operation failed with code " << err << endl;
            }
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Error: Connection lost while creating article." << endl;
        return false;
    }
}

// Perform article fetch using MessageHandler. Returns false if connection lost.
static bool doReadArticle(MessageHandler& mh, int ngId, int aId) {
    try {
        mh.sendCode(Protocol::COM_GET_ART);
        mh.sendIntParameter(ngId);
        mh.sendIntParameter(aId);
        mh.sendCode(Protocol::COM_END);

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_GET_ART)) {
            throw ProtocolViolationException("Expected ANS_GET_ART");
        }

        int status = mh.recvCode();
        if (status == static_cast<int>(Protocol::ANS_ACK)) {
            string title = mh.recvStringParameter();
            string author = mh.recvStringParameter();
            string text = mh.recvStringParameter();
            cout << "Title: " << title << endl;
            cout << "Author: " << author << endl;
            cout << "Text: " << text << endl;
        } else {
            int err = mh.recvCode();
            if (err == static_cast<int>(Protocol::ERR_NG_DOES_NOT_EXIST)) {
                cout << "Error: Newsgroup " << ngId << " does not exist." << endl;
            } else if (err == static_cast<int>(Protocol::ERR_ART_DOES_NOT_EXIST)) {
                cout << "Error: Article " << aId << " does not exist." << endl;
            } else {
                cout << "Error: Operation failed with code " << err << endl;
            }
        }

        if (mh.recvCode() != static_cast<int>(Protocol::ANS_END)) {
            throw ProtocolViolationException("Expected ANS_END");
        }
        return true;
    } catch (const ConnectionClosedException&) {
        cerr << "Error: Connection lost while creating article." << endl;
        return false;
    }
}

/*
 * Send an integer to the server as four bytes.
 */
void writeNumber(const Connection &conn, int value)
{
    conn.write((value >> 24) & 0xFF);
    conn.write((value >> 16) & 0xFF);
    conn.write((value >> 8) & 0xFF);
    conn.write(value & 0xFF);
}

/*
 * Read a string from the server.
 */
string readString(const Connection &conn)
{
    string s;
    char ch;
    while ((ch = conn.read()) != '$')
    {
        s += ch;
    }
    return s;
}

/* Creates a client for the given args, if possible.
 * Otherwise exits with error code.
 */
Connection init(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: myclient host-name port-number" << endl;
        exit(1);
    }

    int port = -1;
    try
    {
        port = std::stoi(argv[2]);
    }
    catch (std::exception &e)
    {
        cerr << "Wrong port number. " << e.what() << endl;
        exit(2);
    }

    Connection conn(argv[1], port);
    if (!conn.isConnected())
    {
        cerr << "Connection attempt failed" << endl;
        exit(3);
    }

    return conn;
}

enum class Command {
    LIST,
    CREATE,
    DELETE,
    READ,
    HELP,
    QUIT,
    UNKNOWN
};

static Command parseCommand(const std::string& s) {
    if (s == "list") return Command::LIST;
    if (s == "create") return Command::CREATE;
    if (s == "delete") return Command::DELETE;
    if (s == "read") return Command::READ;
    if (s == "help") return Command::HELP;
    if (s == "quit") return Command::QUIT;
    return Command::UNKNOWN;
}

int app(Connection& conn)
{
    MessageHandler mh(conn, false);
    string command;
    while (cout << "$ " && cin >> command) {
        try {
            Command parsedCommand = parseCommand(command);
            switch (parsedCommand) {
                case Command::LIST: {
                    // List newsgroups or articles in a newsgroup
                    string rest;
                    std::getline(cin, rest); // remainder of the line after 'create'
                    std::istringstream iss(rest);
                    string ngToken;

                    if (readToken(iss, ngToken)) {
                        string extra;
                        if (readToken(iss, extra)) {
                            cout << "Usage: list [newsgroup_id]" << endl;
                            continue;
                        }
                        int ngId;
                        try {
                            ngId = std::stoi(ngToken);
                        } catch (...) {
                            cout << "Newsgroup ID must be a number" << endl;
                            continue;
                        }
                        if (!doListArticles(mh, ngId)) return 1;
                    } else {
                        if (!doListNewsgroups(mh)) return 1;
                    }
                }
                break;
                case Command::CREATE: {
                    // Newsgroup or article creation
                    // Expect full command on single line: "create ng <name>" or "create a <args...>"
                    string rest;
                    std::getline(cin, rest); // remainder of the line after 'create'
                    std::istringstream iss(rest);
                    string type;
                
                    if (!readToken(iss, type)) {
                        cout << "Usage: create ng <name> | create a <newsgroup_id> <title> <author> <text>" << endl;
                        continue;
                    }

                    if (type == "ng") {
                        string name;
                        if (!readToken(iss, name)) { cout << "Usage: create ng <name>" << endl; continue; }
                        string extra;
                        if (readToken(iss, extra)) { cout << "Please enter one name, or wrap name in quotes if it includes spaces" << endl; continue; }
                        doCreateNG(mh, name);
                    }
                    else if (type == "a") {
                        string idToken, title, author, text;
                        if (!readToken(iss, idToken) || !readToken(iss, title) || !readToken(iss, author) || !readToken(iss, text)) {
                            cout << "Usage: create a <newsgroup_id> <title> <author> <text>" << endl;
                            continue;
                        }
                        string extra;
                        if (readToken(iss, extra)) {
                            cout << "Usage: create a <newsgroup_id> <title> <author> <text>" << endl;
                            continue;
                        }
                        int ngId;
                        try {
                            ngId = std::stoi(idToken);
                        } catch (...) {
                            cout << "Newsgroup ID must be a number" << endl;
                            continue;
                        }
                        doCreateArticle(mh, ngId, title, author, text);
                    } else {
                        cout << "Usage: create ng <name> | create a <newsgroup_id> <title> <author> <text>" << endl;
                    }
                }
                break;
                case Command::DELETE: {
                    // Newsgroup or article deletion
                    // Expect full command on single line: "delete ng <name>" or "delete a <newsgroup_id> <article_id>"
                    string rest;
                    std::getline(cin, rest); // remainder of the line after 'delete'
                    std::istringstream iss(rest);
                    string type;
                
                    if (!readToken(iss, type)) {
                        cout << "Usage: delete ng <newsgroup_id> | delete a <newsgroup_id> <article_id>" << endl;
                        continue;
                    }

                    if (type == "ng") {
                        string idToken;
                        if (!readToken(iss, idToken)) { cout << "Usage: delete ng <newsgroup_id>" << endl; continue; }
                        string extra;
                        if (readToken(iss, extra)) { cout << "Please enter one newsgroup ID" << endl; continue; }
                        int ngId;
                        try {
                            ngId = std::stoi(idToken);
                        } catch (...) {
                            cout << "Newsgroup ID must be a number" << endl;
                            continue;
                        }
                        doDeleteNG(mh, ngId);
                    }
                    else if (type == "a") {
                        string ngIdToken, aIdToken;
                        if (!readToken(iss, ngIdToken) || !readToken(iss, aIdToken)) {
                            cout << "Usage: delete a <newsgroup_id> <article_id>" << endl;
                            continue;
                        }
                        string extra;
                        if (readToken(iss, extra)) {
                            cout << "Usage: delete a <newsgroup_id> <article_id>" << endl;
                            continue;
                        }
                        int ngId;
                        int aId;
                        try {
                            ngId = std::stoi(ngIdToken);
                        } catch (...) {
                            cout << "Newsgroup ID must be a number" << endl;
                            continue;
                        }
                        try {
                            aId = std::stoi(aIdToken);
                        } catch (...) {
                            cout << "Article ID must be a number" << endl;
                            continue;
                        }
                        doDeleteArticle(mh, ngId, aId);
                    } else {
                        cout << "Usage: delete ng <newsgroup_id> | delete a <newsgroup_id> <article_id>" << endl;
                    }
                }
                break;
                case Command::READ: {
                    // Read an article from a newsgroup
                    string rest;
                    std::getline(cin, rest); // remainder of the line after 'read'
                    std::istringstream iss(rest);
                    string ngIdToken, aIdToken;

                    if (!readToken(iss, ngIdToken) || !readToken(iss, aIdToken)) {
                            cout << "Usage: read <newsgroup_id> <article_id>" << endl;
                            continue;
                        }
                        string extra;
                        if (readToken(iss, extra)) {
                            cout << "Usage: read <newsgroup_id> <article_id>" << endl;
                            continue;
                        }
                        int ngId;
                        int aId;
                        try {
                            ngId = std::stoi(ngIdToken);
                        } catch (...) {
                            cout << "Newsgroup ID must be a number" << endl;
                            continue;
                        }
                        try {
                            aId = std::stoi(aIdToken);
                        } catch (...) {
                            cout << "Article ID must be a number" << endl;
                            continue;
                        }
                        doReadArticle(mh, ngId, aId);
                }
                break;
                case Command::HELP: {
                    cout << "Available commands for the news server:\n" << endl;
                    cout << "List current available newsgroups" << endl;
                    cout << "   list\n" << endl;
                    cout << "List articles in a newsgroup" << endl;
                    cout << "   list [newsgroup id]\n" << endl;
                    cout << "Create a newsgroup" << endl;
                    cout << "   create ng [name]\n" << endl;
                    cout << "Delete a newsgroup" << endl;
                    cout << "   delete ng [name]\n" << endl;
                    cout << "List articles in a newsgroup" << endl;
                    cout << "   list [name]\n" << endl;
                    cout << "Create an article" << endl;
                    cout << "   create a [newsgroup id] [title] [author] [text]\n" << endl;
                    cout << "Delete an article" << endl;
                    cout << "   delete a [newsgroup id] [article id]\n" << endl;
                    cout << "Get an article" << endl;
                    cout << "   get [newsgroup id] [title]\n" << endl;
                    cout << "Quit" << endl;
                    cout << "   quit\n" << endl;
                }
                break;
                case Command::QUIT: {
                    cout << "quit command received" << endl;
                }
                break;
                case Command::UNKNOWN: {
                    cout << "Unknown command. Type help for help" << endl;
                    // Consume remainder of the input line so stray tokens aren't read as next command
                    string _rest; std::getline(cin, _rest);
                }
                break;
            }
        } catch (const ConnectionClosedException&) {
            cerr << "Server disconnected." << endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    try
    {
        // Establish a connection to the host
        Connection conn = init(argc, argv);

        // Starts the main client application loop
        return app(conn);
    }
    catch (const std::exception& e)
    {
        // Handle unrecoverable errors with informative messages
        cerr << "Unrecoverable error: " << e.what() << endl;
        return 1;
    }
}
