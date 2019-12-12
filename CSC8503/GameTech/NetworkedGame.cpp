#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

NetworkedGame::NetworkedGame() {
    TutorialGame();
}

NetworkedGame::~NetworkedGame() {
    
}

void NetworkedGame::StartAsServer() {
    
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
    
}

void NetworkedGame::SpawnPlayer() {
    
}

void NetworkedGame::StartLevel() {
    
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
    
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
    
}

void NetworkedGame::UpdateAsServer(float dt) {
    
}

void NetworkedGame::UpdateAsClient(float dt) {
    ClientPacket newPacket;

    // Do update

    m_ThisClient->SendPacket(newPacket);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    
    m_World->GetObjectIterators(first, last);
    
    for (auto i = first; i != last; ++i) {
        NetworkObject* o = (*i)->GetNetworkObject();
        if (!o) {
            continue;
        }

        int playerState = 0; // You ’ll need to do this bit !
        GamePacket * newPacket = nullptr;
        if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
            m_ThisServer->SendGlobalPacket(*newPacket); // change ...
            delete newPacket;
        }
    }
}

void NetworkedGame::UpdateMinimumState() {
    
}