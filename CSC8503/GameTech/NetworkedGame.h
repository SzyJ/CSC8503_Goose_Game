#pragma once
#include "TutorialGame.h"

namespace NCL {
    namespace CSC8503 {
        class GameServer;

        class GameClient;

        class NetworkPlayer;

        enum class KeysPressed {
            W = 1,
            A = 2,
            S = 4,
            D = 8,
            Space = 16
        };

        class NetworkedGame : public TutorialGame, public PacketReceiver {
        public:
            NetworkedGame();
            virtual ~NetworkedGame();

            bool StartAsServer();
            bool StartAsClient(char a, char b, char c, char d);

            void UpdateGame(float dt) override;

            void SpawnPlayer(int playerID);

            void ReceivePacket(int type, GamePacket* payload, int source) override;

            void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

        protected:
            void ClientReceiver(int type, GamePacket* payload, int source);
            void ServerReceiver(int type, GamePacket* payload, int source);

            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);

            void BroadcastSnapshot(bool deltaFrame);
            void UpdateMinimumState();

            std::map<int, int> m_StateIDs;

            bool m_IsServer = false;

            GameServer* m_ThisServer = nullptr;
            GameClient* m_ThisClient = nullptr;
            float m_TimeToNextPacket;
            int m_PacketsToSnapshot;
            std::vector<NetworkObject*> m_NetworkObjects;
            std::map<int, GameObject*> m_ServerPlayers;
            GameObject* m_LocalPlayer = nullptr;

            int m_ThisClientPlayerID;
        };
    }
}
