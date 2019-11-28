#pragma once

#include "../../Common/Matrix4.h"
#include "../../Common/Matrix3.h"
#include "../../Common/Vector3.h"
#include "../../Common/Quaternion.h"

#include <vector>

using std::vector;
using namespace NCL::Maths;

namespace NCL {
    namespace CSC8503 {

        class Transform {
        public:
            Transform();
            Transform(const Vector3& position, Transform* parent = nullptr);
            ~Transform() = default;

            void SetWorldPosition(const Vector3& worldPos);
            void SetLocalPosition(const Vector3& localPos);

            void SetWorldScale(const Vector3& worldScale);
            void SetLocalScale(const Vector3& localScale);

            Transform* GetParent() const {
                return m_Parent;
            }

            void SetParent(Transform* newParent) {
                m_Parent = newParent;
            }

            Matrix4 GetWorldMatrix() const {
                return m_WorldMatrix;
            }

            Matrix4 GetLocalMatrix() const {
                return m_LocalMatrix;
            }

            Vector3 GetWorldPosition() const {
                return m_WorldMatrix.GetPositionVector();
            }

            Vector3 GetLocalPosition() const {
                return m_LocalPosition;
            }

            Vector3 GetLocalScale() const {
                return m_LocalScale;
            }

            Quaternion GetLocalOrientation() const {
                return m_LocalOrientation;
            }

            void SetLocalOrientation(const Quaternion& newOr) {
                m_LocalOrientation = newOr;
            }

            Quaternion GetWorldOrientation() const {
                return m_WorldOrientation;
            }

            Matrix3 GetInverseWorldOrientationMat() const {
                return m_WorldOrientation.Conjugate().ToMatrix3();
            }

            void UpdateMatrices();

        protected:
            Matrix4 m_LocalMatrix;
            Matrix4 m_WorldMatrix;
            Vector3 m_LocalPosition;
            Vector3 m_LocalScale;
            Quaternion m_LocalOrientation;
            Quaternion m_WorldOrientation;
            Transform* m_Parent;
            vector<Transform*> m_Children;
        };

    }
}
