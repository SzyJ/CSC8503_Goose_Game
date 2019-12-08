#pragma once

#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

#include <vector>
#include <deque>

using std::vector;

namespace NCL {
    namespace CSC8503 {

        class NetworkObject;
        class GameObject {
        public:
            GameObject(string objectName = "")
                : m_Name(objectName), m_IsActive(true) { }

            virtual ~GameObject() {
                delete m_BoundingVolume;
                delete m_PhysicsObject;
                delete m_RenderObject;
                delete m_NetworkObject;
            }

            void SetBoundingVolume(CollisionVolume* vol) {
                m_BoundingVolume = vol;
            }

            const CollisionVolume* GetBoundingVolume() const {
                return m_BoundingVolume;
            }

            bool IsActive() const {
                return m_IsActive;
            }

            bool IsSleeping() const {
                return m_IsSleeping;
            }

            void MakeActive() {
                m_IsActive = true;
            }

            void SetSleep(bool sleepState) {
                m_IsSleeping = sleepState;

                if (!sleepState) {
                    m_PositionDeltaQueue.clear();
                }
            }

            const Transform& GetConstTransform() const {
                return m_Transform;
            }

            Transform& GetTransform() {
                return m_Transform;
            }

            RenderObject* GetRenderObject() const {
                return m_RenderObject;
            }

            PhysicsObject* GetPhysicsObject() const {
                return m_PhysicsObject;
            }

            NetworkObject* GetNetworkObject() const {
                return m_NetworkObject;
            }

            void AddPositionDelta(const Vector3& positionDelta) {
                m_PositionDeltaQueue.push_front(positionDelta);
                Vector3 posDeltaAvg = Vector3(0.0f, 0.0f, 0.0f);

                for (Vector3 posDelta : m_PositionDeltaQueue) {
                    posDeltaAvg += posDelta;
                }

                const size_t queueSize = m_PositionDeltaQueue.size();
                
                if (queueSize >= 5) {
                    posDeltaAvg /= queueSize;
                    //m_IsSleeping = posDeltaAvg.GetAbsMaxElement() < 0.5f;

                    m_PositionDeltaQueue.pop_back();
                }
            }

            void SetRenderObject(RenderObject* newObject) {
                m_RenderObject = newObject;
            }

            void SetPhysicsObject(PhysicsObject* newObject) {
                m_PhysicsObject = newObject;
            }

            const string& GetName() const {
                return m_Name;
            }

            virtual void OnCollisionBegin(GameObject* otherObject) {
                //std::cout << "OnCollisionBegin event occured!\n";
            }

            virtual void OnCollisionEnd(GameObject* otherObject) {
                //std::cout << "OnCollisionEnd event occured!\n";
            }

            bool GetBroadphaseAABB(Vector3& outsize) const;

            void UpdateBroadphaseAABB();

        protected:
            Transform m_Transform;
            CollisionVolume* m_BoundingVolume = nullptr;
            PhysicsObject* m_PhysicsObject = nullptr;
            RenderObject* m_RenderObject = nullptr;
            NetworkObject* m_NetworkObject = nullptr;
            bool m_IsActive;
            string m_Name;
            Vector3 m_BroadphaseAABB;

            std::deque<Vector3> m_PositionDeltaQueue;
            bool m_IsSleeping = false;
        };

    }
}
