#pragma once

#include "../CSC8503Common/GameWorld.h"
#include <set>

namespace NCL {
    namespace CSC8503 {

        class PhysicsSystem {
        public:
            PhysicsSystem(GameWorld& g)
                : m_GameWorld(g), m_DTOffset(0.0f), m_GlobalDamping(0.95f) {
                SetGravity(Vector3(0.0f, -9.81f, 0.0f));
            }
            ~PhysicsSystem() = default;

            void Clear();

            void Update(float dt);

            void UseGravity(bool state) {
                m_ApplyGravity = state;

                std::vector<GameObject*>::const_iterator first;
                std::vector<GameObject*>::const_iterator last;
                m_GameWorld.GetObjectIterators(first, last);

                for (auto i = first; i != last; ++i) {
                    (*i)->GetPhysicsObject()->SetSleep(false);
                }
            }

            void SetGlobalDamping(float d) {
                m_GlobalDamping = d;
            }

            void SetGravity(const Vector3& g);

        protected:
            void BasicCollisionDetection();
            void BroadPhase();
            void NarrowPhase();

            void ClearForces();

            void IntegrateAccel(float dt);
            void IntegrateVelocity(float dt);

            void UpdateConstraints(float dt);

            void UpdateCollisionList();
            void UpdateObjectAABBs();

            void UpdateDebugColours();

            void ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;

            GameWorld& m_GameWorld;

            bool m_ApplyGravity = true;

            Vector3 m_Gravity;

            float m_DTOffset;
            float m_GlobalDamping;
            float m_FrameDT;

            std::set<CollisionDetection::CollisionInfo> m_AllCollisions;
            std::set<CollisionDetection::CollisionInfo> m_BroadphaseCollisions;
            std::vector<CollisionDetection::CollisionInfo> m_BroadphaseCollisionsVec;

            bool m_UseBroadPhase = true;

            int m_NumCollisionFrames = 5;
        };

    }
}
