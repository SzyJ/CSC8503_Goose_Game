#pragma once
#include <winsock2.h>
#include <enet/enet.h>
#include <map>
#include <string>

enum BasicNetworkMessages {
    None,
    Hello,
    Message,
    String_Message,
    Delta_State,
    //1 byte per channel since the last state
    Full_State,
    //Full transform etc
    Received_State,
    //received from a client, informs that its received packet n
    Player_Connected,
    Player_Disconnected,
    Shutdown
};

struct GamePacket {
    short Size;
    short Type;

    GamePacket() {
        Type = BasicNetworkMessages::None;
        Size = 0;
    }

    GamePacket(short type) : GamePacket() {
        this->Type = type;
    }

    int GetTotalSize() {
        return sizeof(GamePacket) + Size;
    }
};

struct StringPacket : public GamePacket {
    char StringData[256];

    StringPacket(const std::string& message) {
        Type = BasicNetworkMessages::String_Message;
        Size = (short) message.length();

        memcpy(StringData, message.data(), Size);
    };

    std::string GetStringFromData() {
        std::string realString(StringData);
        realString.resize(Size);
        return realString;
    }
};

struct NewPlayerPacket : public GamePacket {
    int PlayerID;

    NewPlayerPacket(int p) {
        Type = BasicNetworkMessages::Player_Connected;
        PlayerID = p;
        Size = sizeof(int);
    }
};

struct PlayerDisconnectPacket : public GamePacket {
    int PlayerID;

    PlayerDisconnectPacket(int p) {
        Type = BasicNetworkMessages::Player_Disconnected;
        PlayerID = p;
        Size = sizeof(int);
    }
};

class PacketReceiver {
public:
    virtual void ReceivePacket(int type, GamePacket* payload, int source = -1) = 0;
};

class NetworkBase {
public:
    static void Initialise();
    static void Destroy();

    static int GetDefaultPort() {
        return 1234;
    }

    void RegisterPacketHandler(int msgID, PacketReceiver* receiver) {
        m_PacketHandlers.insert(std::make_pair(msgID, receiver));
    }

protected:
    NetworkBase() = default;
    ~NetworkBase();

    bool ProcessPacket(GamePacket* p, int peerID = -1);

    typedef std::multimap<int, PacketReceiver*>::const_iterator PacketHandlerIterator;

    bool GetPacketHandlers(int msgID, PacketHandlerIterator& first, PacketHandlerIterator& last) const {
        auto range = m_PacketHandlers.equal_range(msgID);

        if (range.first == m_PacketHandlers.end()) {
            return false; //no handlers for this message type!
        }
        first = range.first;
        last = range.second;
        return true;
    }

    ENetHost* m_NetHandle = nullptr;

    std::multimap<int, PacketReceiver*> m_PacketHandlers;
};
