#pragma once

#include "CollisionVolume.h"
#include "../../Common/Vector3.h"

namespace NCL {

    class AABBVolume : CollisionVolume {
    public:
        AABBVolume(const Vector3& halfDims)
            : m_HalfSizes(halfDims) {
            Type = VolumeType::AABB;
        }
        ~AABBVolume() = default;

        Vector3 GetHalfDimensions() const {
            return m_HalfSizes;
        }

    protected:
        Vector3 m_HalfSizes;
    };

}
