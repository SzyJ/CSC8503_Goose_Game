#pragma once

#include "..\CSC8503Common\GameObject.h"
#include "..\CSC8503Common\GameClient.h"

namespace NCL {
    namespace CSC8503 {

        class NetworkedGame;

        class NetworkPlayer : public GameObject {
        public:
            NetworkPlayer(NetworkedGame* game, int num)
                : m_Game(game), m_PlayerNum(num) { }
            virtual ~NetworkPlayer() = default;


            void OnCollisionBegin(GameObject* otherObject) override;

            int GetPlayerNum() const {
                return m_PlayerNum;
            }

        protected:
            NetworkedGame* m_Game = nullptr;
            int m_PlayerNum;
        };

    }
}
