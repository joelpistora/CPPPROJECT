cmake_minimum_required(VERSION 3.14)
target_sources(clientserver
    PRIVATE
        connection.cc
        inmemorydatabase.cc
        server.cc
        newsserver.cc
        messageHandler.cc
)
