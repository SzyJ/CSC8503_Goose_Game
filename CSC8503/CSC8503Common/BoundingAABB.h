#pragma once

#include "BoundingVolume.h"

namespace NCL {

    class BoundingAABB : BoundingVolume {
    public:
        BoundingAABB(const Vector3& halfDims)
            : m_HalfSizes(halfDims) {
            m_Type = BoundingType::AABB;
        }
        ~BoundingAABB() = default;

        Vector3 GetHalfDimensions() const {
            return m_HalfSizes;
        }

    protected:
        Vector3 m_HalfSizes;
    };

}
