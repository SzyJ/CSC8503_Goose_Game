#pragma once
#include "TutorialGame.h"

namespace NCL {
    namespace CSC8503 {
        class GameServer;

        class GameClient;

        class NetworkPlayer;

        class NetworkedGame : public TutorialGame, public PacketReceiver {
        public:
            NetworkedGame();
            virtual ~NetworkedGame();

            void StartAsServer();
            void StartAsClient(char a, char b, char c, char d);

            void UpdateGame(float dt) override;

            void SpawnPlayer();

            void StartLevel();

            void ReceivePacket(int type, GamePacket* payload, int source) override;

            void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

        protected:
            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);

            void BroadcastSnapshot(bool deltaFrame);
            void UpdateMinimumState();

            std::map<int, int> m_StateIDs;

            GameServer* m_ThisServer = nullptr;
            GameClient* m_ThisClient = nullptr;
            float m_TimeToNextPacket;
            int m_PacketsToSnapshot;
            std::vector<NetworkObject*> m_NetworkObjects;
            std::map<int, GameObject*> m_ServerPlayers;
            GameObject* m_LocalPlayer = nullptr;
        };
    }
}
