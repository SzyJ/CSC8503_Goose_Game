#pragma once

#include "../../Common/Vector3.h"
#include "../../Common/Quaternion.h"

namespace NCL {

    using namespace Maths;

    namespace CSC8503 {

        class GameObject;
        class NetworkState {
        public:
            NetworkState()
                : m_StateID(0) { }
            virtual ~NetworkState() = default;

            Vector3 m_Position;
            Quaternion m_Orientation;
            int m_StateID;
        };

    }
}
