cmake_minimum_required(VERSION 3.14)
target_sources(clientserver
    PRIVATE
        connection.cc
        diskdatabase.cc
        inmemorydatabase.cc
        server.cc
        messageHandler.cc
)
