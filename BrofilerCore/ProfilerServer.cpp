#include "Common.h"
#include "ProfilerServer.h"

#include "Socket.h"
#include "Message.h"

#if MT_PLATFORM_WINDOWS
#   pragma comment( lib, "ws2_32.lib" )
#else
#   error Platform is not defined!
#endif

namespace Brofiler {

////////////////////////////////////////////////////////////
//
//    Constants
//
/////

constexpr short DEFAULT_PORT = 31313;


////////////////////////////////////////////////////////////
//
//    Server
//
/////

Server::Server (short port)
    : socket(new Socket())
    , isInitialized(false)
{
    socket->Bind(port, 8);
    socket->Listen();
}

void Server::Update () {
    MT::ScopedGuard guard(lock);

    InitConnection();

    int length = -1;
    while ((length = socket->Receive(buffer, BIFFER_SIZE)) > 0) {
        networkStream.Append(buffer, length);
    }

    while (IMessage * message = IMessage::Create(networkStream)) {
        message->Apply();
        delete message;
    }
}

void Server::Send (DataResponse::Type type, OutputDataStream & stream) {
    MT::ScopedGuard guard(lock);

    std::string data = stream.GetData();

    DataResponse response (type, (uint32)data.size());
    socket->Send((char *)&response, sizeof(response));
    socket->Send(data.c_str(), data.size());
}

bool Server::InitConnection () {
    if (!isInitialized) {
        acceptThread.Start(1 * 1024 * 1024, Server::AsyncAccept, this);
        isInitialized = true;
        return true;
    }
    return false;
}

Server::~Server () {
    acceptThread.Join();

    if (socket) {
        delete socket;
        socket = nullptr;
    }
}

Server & Server::Get () {
    static Server instance(DEFAULT_PORT);
    return instance;
}

bool Server::Accept () {
    socket->Accept();
    return true;
}

void Server::AsyncAccept (void * _server) {
    Server * server = (Server *)_server;

    while (server->Accept()) {
        MT::Thread::Sleep(1000);
    }
}

} // Brofiler
