#include "GameServer.h"
#include "GameWorld.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)
    : m_Port(onPort), m_ClientMax(maxClients), m_ClientCount(0) {

    //m_ThreadAlive = false;
    Initialise();
}

GameServer::~GameServer() {
    Shutdown();
}

void GameServer::Shutdown() {
    SendGlobalPacket(BasicNetworkMessages::Shutdown);

    //m_ThreadAlive = false;
    //m_UpdateThread.join();

    enet_host_destroy(m_NetHandle);
    m_NetHandle = nullptr;
}

bool GameServer::Initialise() {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = m_Port;

    m_NetHandle = enet_host_create(&address, m_ClientMax, 1, 0, 0);

    if (!m_NetHandle) {
        std::cout << __FUNCTION__ << " failed to create network handle!" << std::endl;
        return false;
    }
    //m_ThreadAlive = true;
    //m_UpdateThread = std::thread(&GameServer::ThreadedUpdate, this);

    return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
    GamePacket packet;
    packet.Type = msgID;

    return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
    ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
    enet_host_broadcast(m_NetHandle, 0, dataPacket);
    return true;
}

void GameServer::UpdateServer() {
    if (!m_NetHandle) {
        return;
    }

    ENetEvent event;
    while (enet_host_service(m_NetHandle, &event, 0) > 0) {
        int type = event.type;
        ENetPeer* p = event.peer;

        int peer = p->incomingPeerID;

        if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Server: New client connected" << std::endl;
            NewPlayerPacket player(peer);
            SendGlobalPacket(player);
        } else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
            std::cout << "Server: A client has disconnected" << std::endl;
            PlayerDisconnectPacket player(peer);
            SendGlobalPacket(player);
        } else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
            GamePacket* packet = (GamePacket*) event.packet->data;
            ProcessPacket(packet, peer);
        }
        enet_packet_destroy(event.packet);
    }
}

//void GameServer::ThreadedUpdate() {
//    while (m_ThreadAlive) {
//        UpdateServer();
//    }
//}

//Second networking tutorial stuff

void GameServer::SetGameWorld(GameWorld& g) {
    m_GameWorld = &g;
}
