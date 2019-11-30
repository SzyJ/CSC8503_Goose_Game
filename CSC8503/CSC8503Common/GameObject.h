#pragma once

#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

#include <vector>

using std::vector;

namespace NCL {
    namespace CSC8503 {

        class NetworkObject;
        class GameObject {
        public:
           GameObject(string objectName = "")
                : m_Name(objectName), m_IsActive(true) {
                m_Name = objectName;
                m_IsActive = true;
            }

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
        };

    }
}