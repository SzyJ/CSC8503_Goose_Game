#pragma once

#include <thread>
#include <atomic>

#include "NetworkBase.h"

namespace NCL {
    namespace CSC8503 {

        class GameWorld;
        class GameServer : public NetworkBase {
        public:
            GameServer(int onPort, int maxClients);
            ~GameServer();

            bool Initialise();
            void Shutdown();

            void SetGameWorld(GameWorld& g);

            //void ThreadedUpdate();

            bool SendGlobalPacket(int msgID);
            bool SendGlobalPacket(GamePacket& packet);

            virtual void UpdateServer();

        protected:
            int m_Port;
            int m_ClientMax;
            int m_ClientCount;
            GameWorld* m_GameWorld = nullptr;
            //std::atomic<bool> m_ThreadAlive;
            //std::thread m_UpdateThread;
            int m_IncomingDataRate;

            int m_OutgoingDataRate;
        };
    }
}
