#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

bool GameObject::GetBroadphaseAABB(Vector3& outSize) const {
    if (!m_BoundingVolume) {
        return false;
    }
    outSize = m_BroadphaseAABB;
    return true;
}

//These would be better as a virtual 'ToAABB' type function, really...
void GameObject::UpdateBroadphaseAABB() {
    if (!m_BoundingVolume) {
        return;
    }
    if (m_BoundingVolume->Type == VolumeType::AABB) {
        m_BroadphaseAABB = ((AABBVolume&) *m_BoundingVolume).GetHalfDimensions();
    } else if (m_BoundingVolume->Type == VolumeType::Sphere) {
        float r = ((SphereVolume&) *m_BoundingVolume).GetRadius();
        m_BroadphaseAABB = Vector3(r, r, r);
    } else if (m_BoundingVolume->Type == VolumeType::OBB) {
        Matrix3 mat = Matrix3(m_Transform.GetWorldOrientation());
        mat = mat.Absolute();
        Vector3 halfSizes = ((OBBVolume&) *m_BoundingVolume).GetHalfDimensions();
        m_BroadphaseAABB = mat * halfSizes;
    }
}
