#pragma once

#include "BoundingVolume.h"

namespace NCL {

    class BoundingSphere : BoundingVolume {
    public:
        BoundingSphere(float radius = 1.0f)
            : m_Radius(radius) {
            m_Type = BoundingType::Sphere;
        }
        ~BoundingSphere() = default;

        float GetRadius() const {
            return m_Radius;
        }

        Vector3 m_Origin;
        float m_Radius;
    };

}
