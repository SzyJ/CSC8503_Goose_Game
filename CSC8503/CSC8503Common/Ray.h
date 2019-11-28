#pragma once

#include "../../Common/Vector3.h"
#include "../../Common/Plane.h"

namespace NCL {
    namespace Maths {

        struct RayCollision {
            void* Node; //Node that was hit
            Vector3 CollidedAt; //WORLD SPACE position of the collision!
            float RayDistance;

            RayCollision(void* node, Vector3 collidedAt)
                : Node(node), CollidedAt(collidedAt), RayDistance(0.0f) { }
            RayCollision()
                : Node(nullptr), RayDistance(FLT_MAX) { }
        };

        class Ray {
        public:
            Ray(Vector3 position, Vector3 direction)
                : m_Position(position), m_Direction(direction) { }
            ~Ray() = default;

            Vector3 GetPosition() const { return m_Position; }

            Vector3 GetDirection() const { return m_Direction; }

        protected:
            Vector3 m_Position; //World space position
            Vector3 m_Direction; //Normalised world space direction
        };

    }
}
