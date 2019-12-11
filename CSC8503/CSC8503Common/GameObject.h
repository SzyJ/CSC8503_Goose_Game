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
                return m_PhysicsObject->IsSleeping();
            }

            void MakeActive() {
                m_IsActive = true;
            }

            void SetSleep(bool sleepState) {
                m_PhysicsObject->SetSleep(sleepState);
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
                m_PhysicsObject->AddPositionDelta(positionDelta);
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
                if (m_Name == "Water") {
                    Vector3 otherPosition = otherObject->GetConstTransform().GetWorldPosition();

                    const float heightDelta = otherPosition.y - m_Transform.GetWorldPosition().y;

                    const float floatOffset = m_Transform.GetLocalScale().y + 1.0f;

                    const float forceStrength = (10.0f * 5.0f) - (5.0f * 1.0f);
                    otherObject->GetPhysicsObject()->AddForce(Vector3(0.0f, heightDelta - floatOffset, 0.0f) * -forceStrength);

                    const float hSlowdown = 0.1f;
                    const float vSlowdown = 0.0f;
                    const Vector3 slowdownFactor(hSlowdown, vSlowdown, hSlowdown);
                    const Vector3 currentForce = otherObject->GetPhysicsObject()->GetForce();
                    otherObject->GetPhysicsObject()->AddForce(-currentForce * slowdownFactor);

                    if (otherObject->GetName() == "Keeper") {
                        Vector3 swimDir(0.0f, 1.0f, 0.0f);

                        otherObject->GetPhysicsObject()->AddForce(swimDir * 50.0f);
                    }
                }
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
        };

    }
}
