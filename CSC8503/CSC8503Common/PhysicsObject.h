#pragma once

#include "../../Common/Vector3.h"
#include "../../Common/Matrix3.h"

using namespace NCL::Maths;

namespace NCL {
    class CollisionVolume;

    namespace CSC8503 {
        class Transform;

        class PhysicsObject {
        public:
            PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume)
                :m_Volume(parentVolume), m_Transform(parentTransform), m_InverseMass(1.0f), m_Elasticity(0.8f), m_Friction(0.8f) { }

            ~PhysicsObject() = default;

            Vector3 GetLinearVelocity() const {
                return m_LinearVelocity;
            }

            Vector3 GetAngularVelocity() const {
                return m_AngularVelocity;
            }

            Vector3 GetTorque() const {
                return m_Torque;
            }

            Vector3 GetForce() const {
                return m_Force;
            }

            void SetInverseMass(float invMass) {
                m_InverseMass = invMass;
            }

            float GetInverseMass() const {
                return m_InverseMass;
            }

            void ApplyAngularImpulse(const Vector3& force);
            void ApplyLinearImpulse(const Vector3& force);

            void AddForce(const Vector3& force);

            void AddForceAtPosition(const Vector3& force, const Vector3& position);

            void AddTorque(const Vector3& torque);


            void ClearForces();

            void SetLinearVelocity(const Vector3& v) {
                m_LinearVelocity = v;
            }

            void SetAngularVelocity(const Vector3& v) {
                m_AngularVelocity = v;
            }

            void InitCubeInertia();
            void InitSphereInertia();

            void UpdateInertiaTensor();

            Matrix3 GetInertiaTensor() const {
                return m_InverseInteriaTensor;
            }

        protected:
            const CollisionVolume* m_Volume;
            Transform* m_Transform = nullptr;
            float m_InverseMass;
            float m_Elasticity;
            float m_Friction;

            //linear stuff
            Vector3 m_LinearVelocity;
            Vector3 m_Force;

            //angular stuff
            Vector3 m_AngularVelocity;
            Vector3 m_Torque;
            Vector3 m_InverseInertia;
            Matrix3 m_InverseInteriaTensor;
        };
    }
}
