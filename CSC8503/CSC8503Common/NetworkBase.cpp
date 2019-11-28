#include "NetworkBase.h"
#include <enet/enet.h>
#include <iostream>

NetworkBase::~NetworkBase() {
    if (m_NetHandle) {
        enet_host_destroy(m_NetHandle);
    }
}

void NetworkBase::Initialise() {
    enet_initialize();
}

void NetworkBase::Destroy() {
    enet_deinitialize();
}

bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
    PacketHandlerIterator firstHandler;
    PacketHandlerIterator lastHandler;

    bool canHandle = GetPacketHandlers(packet->Type, firstHandler, lastHandler);

    if (canHandle) {
        for (auto i = firstHandler; i != lastHandler; ++i) {
            i->second->ReceivePacket(packet->Type, packet, peerID);
        }
        return true;
    }
    std::cout << __FUNCTION__ << " no handler for packet type " << packet->Type << std::endl;
    return false;
}
