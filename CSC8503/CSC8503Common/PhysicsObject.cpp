#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "../CSC8503Common/Transform.h"

using namespace NCL;
using namespace CSC8503;

void PhysicsObject::ApplyAngularImpulse(const Vector3& force) {
    if (m_InverseMass - FLT_EPSILON > 0.0f) {
        SetSleep(false);
    }
    m_AngularVelocity += m_InverseInteriaTensor * force;
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force) {
    if (m_InverseMass - FLT_EPSILON > 0.0f) {
        SetSleep(false);
    }

    m_LinearVelocity += force * m_InverseMass;
}

void PhysicsObject::AddForce(const Vector3& addedForce) {
    if (m_InverseMass - FLT_EPSILON > 0.0f) {
        SetSleep(false);
    }

    m_Force += addedForce;
}

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) {
    if (m_InverseMass - FLT_EPSILON > 0.0f) {
        SetSleep(false);
    }
    Vector3 localPos = position - m_Transform->GetWorldPosition();

    AddForce(addedForce);
    m_Torque += Vector3::Cross(localPos, addedForce);
}

void PhysicsObject::AddTorque(const Vector3& addedTorque) {
    if (m_InverseMass - FLT_EPSILON > 0.0f) {
        SetSleep(false);
    }
    m_Torque += addedTorque;
}

void PhysicsObject::ClearForces() {
    m_Force = Vector3();
    m_Torque = Vector3();
}

void PhysicsObject::InitCubeInertia() {
    Vector3 dimensions = m_Transform->GetLocalScale();

    Vector3 fullWidth = dimensions * 2;

    Vector3 dimsSqr = fullWidth * fullWidth;

    m_InverseInertia.x = (12.0f * m_InverseMass) / (dimsSqr.y + dimsSqr.z);
    m_InverseInertia.y = (12.0f * m_InverseMass) / (dimsSqr.x + dimsSqr.z);
    m_InverseInertia.z = (12.0f * m_InverseMass) / (dimsSqr.x + dimsSqr.y);
}

void PhysicsObject::InitSphereInertia() {
    float radius = m_Transform->GetLocalScale().GetMaxElement();
    float i = 2.5f * m_InverseMass / (radius * radius);

    m_InverseInertia = Vector3(i, i, i);
}

void PhysicsObject::UpdateInertiaTensor() {
    Quaternion q = m_Transform->GetWorldOrientation();

    Matrix3 invOrientation = Matrix3(q.Conjugate());
    Matrix3 orientation = Matrix3(q);

    m_InverseInteriaTensor = orientation * Matrix3::Scale(m_InverseInertia) * invOrientation;
}
