#include "PositionConstraint.h"
#include "../../Common/Vector3.h"
#include "GameObject.h"
#include "Debug.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace CSC8503;

//a simple constraint that stops objects from being more than <m_Distance> away
//from each other...this would be all we need to simulate a rope, or a ragdoll
void PositionConstraint::UpdateConstraint(float dt) {
    const Vector3 relPosition = m_ObjectA->GetConstTransform().GetWorldPosition() - m_ObjectB->GetConstTransform().GetWorldPosition();
    const float currentDistance = relPosition.Length();
    const float offset = m_Distance - currentDistance;

    if (abs(offset) < 0.0f) {
        return;
    }

    const Vector3 offsetDir = relPosition.Normalised();
    PhysicsObject* aPhysObj = m_ObjectA->GetPhysicsObject();
    PhysicsObject* bPhysObj = m_ObjectB->GetPhysicsObject();

    const Vector3 relVelocity = aPhysObj->GetLinearVelocity() - bPhysObj->GetLinearVelocity();
    const float constraintMass = aPhysObj->GetInverseMass() + bPhysObj->GetInverseMass();

    if (constraintMass < 0.0f) {
        return;
    }

    const float velocityDot = relVelocity.Dot(offsetDir);
    const float biasFactor = 0.01f;
    const float bias = -(biasFactor / dt) * offset;
    const float lambda = -(velocityDot + bias) / constraintMass;

    const Vector3 aImpulse =  offsetDir * lambda;
    const Vector3 bImpulse = -offsetDir * lambda;

    aPhysObj->ApplyLinearImpulse(aImpulse);
    bPhysObj->ApplyLinearImpulse(bImpulse);
}
