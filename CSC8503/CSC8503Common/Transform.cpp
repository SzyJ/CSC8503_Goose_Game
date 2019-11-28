#include "Transform.h"

using namespace NCL::CSC8503;

Transform::Transform() {
    m_Parent = nullptr;
    m_LocalScale = Vector3(1, 1, 1);
}

Transform::Transform(const Vector3& position, Transform* p) {
    m_Parent = p;
    SetWorldPosition(position);
}

void Transform::UpdateMatrices() {
    m_LocalMatrix = Matrix4::Translation(m_LocalPosition) *
        Matrix4(m_LocalOrientation) *
        Matrix4::Scale(m_LocalScale);

    if (m_Parent) {
        m_WorldMatrix = m_Parent->GetWorldMatrix() * m_LocalMatrix;
        m_WorldOrientation = m_Parent->GetWorldOrientation() * m_LocalOrientation;
    } else {
        m_WorldMatrix = m_LocalMatrix;
        m_WorldOrientation = m_LocalOrientation;
    }
}

void Transform::SetWorldPosition(const Vector3& worldPos) {
    if (m_Parent) {
        Vector3 parentPos = m_Parent->GetWorldMatrix().GetPositionVector();
        Vector3 posDiff = parentPos - worldPos;

        m_LocalPosition = posDiff;
        m_LocalMatrix.SetPositionVector(posDiff);
    } else {
        m_LocalPosition = worldPos;

        m_WorldMatrix.SetPositionVector(worldPos);
    }
}

void Transform::SetLocalPosition(const Vector3& localPos) {
    m_LocalPosition = localPos;
}

void Transform::SetWorldScale(const Vector3& worldScale) {
    if (m_Parent) { } else {
        m_LocalScale = worldScale;
    }
}

void Transform::SetLocalScale(const Vector3& newScale) {
    m_LocalScale = newScale;
}
