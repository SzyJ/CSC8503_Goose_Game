#pragma once

#include "Constraint.h"

namespace NCL {
    namespace CSC8503 {

        class GameObject;

        class PositionConstraint : public Constraint {
        public:
            PositionConstraint(GameObject* a, GameObject* b, float d)
                : m_ObjectA(a), m_ObjectB(b), m_Distance(d) { }
            ~PositionConstraint() = default;

            void UpdateConstraint(float dt) override;

        protected:
            GameObject* m_ObjectA = nullptr;
            GameObject* m_ObjectB = nullptr;

            float m_Distance;
        };

    }
}
