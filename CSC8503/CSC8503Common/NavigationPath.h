#pragma once

#include "../../Common/Vector3.h"
#include <vector>

namespace NCL {
    using namespace NCL::Maths;

    namespace CSC8503 {
        class NavigationPath {
        public:
            NavigationPath() = default;

            ~NavigationPath() = default;

            void Clear() {
                m_Waypoints.clear();
            }

            void PushWaypoint(const Vector3& wp) {
                m_Waypoints.emplace_back(wp);
            }

            bool PopWaypoint(Vector3& waypoint) {
                if (m_Waypoints.empty()) {
                    return false;
                }
                waypoint = m_Waypoints.back();
                m_Waypoints.pop_back();
                return true;
            }

        protected:
            std::vector<Vector3> m_Waypoints;
        };

    }
}
