#include "GameClient.h"
#include <iostream>
#include <string>

using namespace NCL;
using namespace CSC8503;

GameClient::GameClient() {
    m_NetHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient() {
    //threadAlive = false;
    //updateThread.join();
    enet_host_destroy(m_NetHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
    ENetAddress address;
    address.port = portNum;

    address.host = (d << 24) | (c << 16) | (b << 8) | (a);

    m_NetPeer = enet_host_connect(m_NetHandle, &address, 2, 0);

    //if (m_NetPeer != nullptr) {
        //threadAlive = true;
        //updateThread = std::thread(&GameClient::ThreadedUpdate, this);
    //}

    return m_NetPeer != nullptr;
}

void GameClient::UpdateClient() {
    if (m_NetHandle == nullptr) {
        return;
    }
    //Handle all incoming packets & send any packets awaiting dispatch
    ENetEvent event;
    while (enet_host_service(m_NetHandle, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Client: Connected to server!" << std::endl;
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            std::cout << "Client: Packet recieved..." << std::endl;
            GamePacket* packet = (GamePacket*) event.packet->data;
            ProcessPacket(packet);
        }
        enet_packet_destroy(event.packet);
    }
}

void GameClient::SendPacket(GamePacket& payload) {
    ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);

    enet_peer_send(m_NetPeer, 0, dataPacket);
}

//void GameClient::ThreadedUpdate() {
//    while (threadAlive) {
//        UpdateClient();
//    }
//}
