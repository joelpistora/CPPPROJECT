/* myserver.cc: sample server program */
#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"
#include "newsdatabase.h"
#include "inmemorydatabase.h"
#include "diskdatabase.h"
#include "newsmodel.h"
#include "messageHandler.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;

void handle_list_ng(MessageHandler& mh, NewsDatabase& db)
{
    // THis command takes no parameters so the next byte must be COM_END, otherwise its a violation
    cout << "Handling list newsgroup" << endl;
    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END");
    }

    auto groups = db.listNewsgroups();
    mh.sendCode(Protocol::ANS_LIST_NG);
    mh.sendIntParameter(groups.size());
    for (const auto& ng : groups) {
        mh.sendIntParameter(ng.id);
        mh.sendStringParameter(ng.name);
    }
    mh.sendCode(Protocol::ANS_END);
}

void handle_create_ng(MessageHandler& mh, NewsDatabase& db)
{
    cout << "Creating newsgroup" << endl;
    
    string name = mh.recvStringParameter();

    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END after newsgroup name");
    }

    DbResult result = db.createNewsgroup(name);
    mh.sendCode(Protocol::ANS_CREATE_NG);

    if (result == DbResult::ok) {
        mh.sendCode(Protocol::ANS_ACK);
    } else {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_NG_ALREADY_EXISTS);
    }
    
    mh.sendCode(Protocol::ANS_END);
}

void handle_delete_ng(MessageHandler& mh, NewsDatabase& db)
{
    // COM_DELETE_NG num_p COM_END
    // ANS_DELETE_NG [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END

    cout << "Handling delete newsgroup" << endl;
    
    int newsgroupId = mh.recvIntParameter();

    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END after newgroup ID");
    }

    DbResult res = db.deleteNewsgroup(newsgroupId);

    mh.sendCode(Protocol::ANS_DELETE_NG);

    if (res == DbResult::ok) {
        mh.sendCode(Protocol::ANS_ACK);
    } else {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
    }
    
    mh.sendCode(Protocol::ANS_END);
}

void handle_list_art(MessageHandler& mh, NewsDatabase& db)
{
    // Message: COM_LIST_ART PAR_NUM 0 0 0 1 COM_END

    // Answer: ANS_LIST_ART [ANS_ACK num_p [num_p string_p]* | 
    //              ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
    cout << "Handling list article" << endl;

    int newsgroupId = mh.recvIntParameter();

    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END after newgroup ID");
    }

    ListArticlesResult result = db.listArticles(newsgroupId);

    mh.sendCode(Protocol::ANS_LIST_ART);

    if (result.status == DbResult::ok) {
        mh.sendCode(Protocol::ANS_ACK);
        vector<ArticleSummary> articles = result.articles;
        mh.sendIntParameter(articles.size());
        for (const auto& art : articles) {
            mh.sendIntParameter(art.id);
            mh.sendStringParameter(art.title);
        }
    } else {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
    }

    mh.sendCode(Protocol::ANS_END);
}

void handle_create_art(MessageHandler& mh, NewsDatabase& db)
{
    cout << "Creating article" << endl;
    
    int groupId    = mh.recvIntParameter();
    string title   = mh.recvStringParameter();
    string author  = mh.recvStringParameter();
    string text    = mh.recvStringParameter();

    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END after article text");
    }

    DbResult result = db.createArticle(groupId, title, author, text);
    mh.sendCode(Protocol::ANS_CREATE_ART);

    if (result == DbResult::ok) {
        mh.sendCode(Protocol::ANS_ACK);
    } else {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
    }
    
    mh.sendCode(Protocol::ANS_END);
}

void handle_delete_art(MessageHandler& mh, NewsDatabase& db)
{
    cout << "Handling delete article" << endl;
    
    int newsgroupId = mh.recvIntParameter();
    int articleId = mh.recvIntParameter();

    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END after newgroup ID");
    }

    DbResult res = db.deleteArticle(newsgroupId, articleId);

    mh.sendCode(Protocol::ANS_DELETE_ART);

    if (res == DbResult::group_not_found) {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
    } else if (res == DbResult::article_not_found) {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_ART_DOES_NOT_EXIST);
    } else {
        mh.sendCode(Protocol::ANS_ACK);
    }
    
    mh.sendCode(Protocol::ANS_END);
}

void handle_get_art(MessageHandler& mh, NewsDatabase& db)
{
    cout << "Handling get article" << endl;
    
    int newsgroupId = mh.recvIntParameter();
    int articleId = mh.recvIntParameter();

    if (mh.recvCode() != static_cast<int>(Protocol::COM_END)) {
        throw ProtocolViolationException("Expected COM_END after newgroup ID");
    }

    GetArticleResult res = db.getArticle(newsgroupId, articleId);

    mh.sendCode(Protocol::ANS_GET_ART);

    if (res.status == DbResult::ok) {
        Article article = res.article;
        mh.sendCode(Protocol::ANS_ACK);
        mh.sendStringParameter(article.title);
        mh.sendStringParameter(article.author);
        mh.sendStringParameter(article.text);
    } else if (res.status == DbResult::group_not_found) {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
    } else {
        mh.sendCode(Protocol::ANS_NAK);
        mh.sendCode(Protocol::ERR_ART_DOES_NOT_EXIST);
    }
    
    mh.sendCode(Protocol::ANS_END);
}

void process_request(std::shared_ptr<Connection>& conn, NewsDatabase& db)
{
    MessageHandler mh(*conn);

    try {
        // Read command code
        int code = mh.recvCode();
        Protocol command = static_cast<Protocol>(code);

        // Run specific handler depending on code
        switch (command) {
            case Protocol::COM_LIST_NG:
                handle_list_ng(mh, db);
                break;
            case Protocol::COM_CREATE_NG:
                handle_create_ng(mh, db);
                break;
            case Protocol::COM_DELETE_NG:
                handle_delete_ng(mh, db);
                break;
            case Protocol::COM_LIST_ART:
                handle_list_art(mh, db);
                break;
            case Protocol::COM_CREATE_ART:
                handle_create_art(mh, db);
                break;
            case Protocol::COM_DELETE_ART:
                handle_delete_art(mh, db);
                break;
            case Protocol::COM_GET_ART:
                handle_get_art(mh, db);
                break;
            default:
                throw ProtocolViolationException("Unknown command");
        }
    } catch (const ProtocolViolationException& e) {
        // If a client violates the protocol, immediately disconnect
        cerr << "Protocol violation: " << e.what() << ". Disconnecting client." << endl;
        throw ConnectionClosedException();
    }
}

Server init(int argc, char* argv[])
{
    if (argc != 2 && argc != 4) {
        cerr << "Usage: myserver port-number [--disk <path>]" << endl;
        exit(1);
    }

    int port = -1;
    try {
        port = std::stoi(argv[1]);
    } catch (std::exception& e) {
        cerr << "Wrong format for port number. " << e.what() << endl;
        exit(2);
    }

    Server server(port);
    if (!server.isReady()) {
        cerr << "Server initialization error." << endl;
        exit(3);
    }
    return server;
}

void serve_one(Server& server, NewsDatabase& db)
{
    auto conn = server.waitForActivity();
    if (conn != nullptr) {
        try {
            process_request(conn, db);
        } catch (ConnectionClosedException&) {
            server.deregisterConnection(conn);
            cout << "Client closed connection" << endl;
        }
    } else {
        conn = std::make_shared<Connection>();
        server.registerConnection(conn);
        cout << "New client connects" << endl;
    }
}

int main(int argc, char* argv[])
{
    
    auto server = init(argc, argv);

    bool onDisk = false;
    std::string diskPath;

    if (argc == 4) {
        std::string flag = argv[2];

        if (flag != "--disk") {
            cerr << "Usage: myserver port-number [--disk <path>]" << endl;
            return 1;

        }
        onDisk = true;

        diskPath = argv[3];
    }

    std::unique_ptr<NewsDatabase> db;

    if (onDisk) {
        db = std::unique_ptr<NewsDatabase>(new DiskDatabase(diskPath));
    } else {
        db = std::unique_ptr<NewsDatabase>(new InMemoryDatabase());
    }

    while (true) {
        serve_one(server, *db);
    }        
}