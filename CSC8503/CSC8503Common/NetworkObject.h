#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"

namespace NCL {
    namespace CSC8503 {
        struct FullPacket : public GamePacket {
            int ObjectID = -1;

            NetworkState FullState;

            FullPacket() {
                Type = Full_State;
                Size = sizeof(FullPacket) - sizeof(GamePacket);
            }
        };

        struct DeltaPacket : public GamePacket {
            int fullID = -1;

            int objectID = -1;

            char pos[3];

            char orientation[4];

            DeltaPacket() {
                Type = Delta_State;
                Size = sizeof(DeltaPacket) - sizeof(GamePacket);
            }
        };

        struct ClientPacket : public GamePacket {
            int LastID;

            char Buttonstates[8];

            ClientPacket() {
                Size = sizeof(ClientPacket);
            }
        };

        class NetworkObject {
        public:
            NetworkObject(GameObject& o, int id)
                : m_Object(o), m_NetworkID(id) {}
            virtual ~NetworkObject() = default;

            //Called by clients
            virtual bool ReadPacket(GamePacket& p);
            //Called by servers
            virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

            void UpdateStateHistory(int minID);

        protected:
            NetworkState& GetLatestNetworkState();

            bool GetNetworkState(int frameID, NetworkState& state);

            virtual bool ReadDeltaPacket(DeltaPacket& p);
            virtual bool ReadFullPacket(FullPacket& p);

            virtual bool WriteDeltaPacket(GamePacket** p, int stateID);
            virtual bool WriteFullPacket(GamePacket** p);

            GameObject& m_Object;
            NetworkState m_LastFullState;
            std::vector<NetworkState> m_StateHistory;

            int m_DeltaErrors = 0;
            int m_FullErrors = 0;
            int m_NetworkID;
        };
    }
}
