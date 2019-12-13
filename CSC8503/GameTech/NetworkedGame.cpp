#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

NetworkedGame::NetworkedGame() {
    NetworkBase::Initialise();

    m_LocalPlayer = m_Goose;
}

NetworkedGame::~NetworkedGame() {
    NetworkBase::Destroy();
}

void NetworkedGame::UpdateGame(float dt) {
    // Do packet stuff
    if (m_IsServer) {
        UpdateAsServer(dt);
    }

    UpdateAsClient(dt);

    // Update game TODO:
    TutorialGame::UpdateGame(dt);

    // TODO send local goose updates to server
    GamePacket* goosePos;
    m_Goose->GetNetworkObject()->WritePacket(&goosePos, false, 0);
}

bool NetworkedGame::StartAsServer() {
    const int port = NetworkBase::GetDefaultPort();

    m_IsServer = true;

    m_ThisServer = new GameServer(port, 1);
    m_ThisServer->RegisterPacketHandler(Delta_State, this);
    m_ThisServer->RegisterPacketHandler(Full_State, this);
    m_ThisServer->RegisterPacketHandler(Player_Connected, this);
    m_ThisServer->RegisterPacketHandler(Player_Disconnected, this);

    m_ThisClient = new GameClient();
    m_ThisClient->RegisterPacketHandler(Delta_State, this);
    m_ThisClient->RegisterPacketHandler(Full_State, this);
    m_ThisClient->RegisterPacketHandler(Player_Connected, this);
    m_ThisClient->RegisterPacketHandler(Player_Disconnected, this);

    bool connectSuccess = m_ThisClient->Connect(127, 0, 0, 1, port);

    int thisID;
    while((thisID = m_ThisClient->GetID()) < 0) {
        m_ThisServer->UpdateServer();
        m_ThisClient->UpdateClient();
    }

    m_LocalPlayer->SetNetworkObject(new NetworkObject(*m_LocalPlayer, thisID));

    return connectSuccess;
}

bool NetworkedGame::StartAsClient(char a, char b, char c, char d) {
    const int port = NetworkBase::GetDefaultPort();

    m_IsServer = false;

    m_ThisClient = new GameClient();
    m_ThisClient->RegisterPacketHandler(Delta_State, this);
    m_ThisClient->RegisterPacketHandler(Full_State, this);
    m_ThisClient->RegisterPacketHandler(Player_Connected, this);
    m_ThisClient->RegisterPacketHandler(Player_Disconnected, this);

    bool connectSuccess = m_ThisClient->Connect(127, 0, 0, 1, port);

    int thisID;
    while ((thisID = m_ThisClient->GetID()) < 0) {
        m_ThisClient->UpdateClient();
    }

    m_LocalPlayer->SetNetworkObject(new NetworkObject(*m_LocalPlayer, thisID));
    return connectSuccess;
}

void NetworkedGame::SpawnPlayer(int playerID) {
    GameObject* newGoose = AddGooseToWorld(m_GameState->GetNavigationGrid()->GetGoosePosition());
    newGoose->GetRenderObject()->SetColour(Vector4(1.0f, 1.0f, 0.0f, 1.0f));
    newGoose->SetNetworkObject(new NetworkObject(*newGoose, playerID));
    m_ServerPlayers.insert(std::pair<int, GameObject*>(playerID, newGoose));
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {

    if (m_IsServer) {
        m_ThisServer->SendGlobalPacket(*payload);
    }
    
    ClientReceiver(type, payload, source);

}

void NetworkedGame::ClientReceiver(int type, GamePacket* payload, int source) {

    switch (type) {
    case Delta_State: {
        DeltaPacket* realPacket = (DeltaPacket*)payload;
        int id = realPacket->ObjectID;

        if (id != m_ThisClientPlayerID) {
            m_ServerPlayers[id]->GetNetworkObject()->ReadPacket(*realPacket);
        }

        break;
    }

    case Full_State: {
        FullPacket* realPacket = (FullPacket*)payload;
        int id = realPacket->ObjectID;

        if (id != m_ThisClientPlayerID) {
            m_ServerPlayers[id]->GetNetworkObject()->ReadPacket(*realPacket);
        }

        break;
    }

    case Player_Connected: {
        NewPlayerPacket* realPacket = (NewPlayerPacket*)payload;
        int thisID = realPacket->PlayerID;
        if (thisID != m_ThisClient->GetID()) {
            SpawnPlayer(realPacket->PlayerID);
        }

        break;
    }

    case Player_Disconnected: {
        PlayerDisconnectPacket* realPacket = (PlayerDisconnectPacket*)payload;
        delete m_ServerPlayers[realPacket->PlayerID];
        m_ServerPlayers.erase(realPacket->PlayerID);

        break;
    }

    default:
        break;
    }
}

void NetworkedGame::UpdateAsServer(float dt) {
    m_ThisServer->UpdateServer();

}

void NetworkedGame::UpdateAsClient(float dt) {
    m_ThisClient->UpdateClient();

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

        GamePacket* newPacket = nullptr;
        if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
            m_ThisServer->SendGlobalPacket(*newPacket); // change ...
            delete newPacket;
        }
    }
}

void NetworkedGame::UpdateMinimumState() {
    
}