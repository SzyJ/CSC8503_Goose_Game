#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"

namespace NCL {
    class OBBVolume : CollisionVolume {
    public:
        OBBVolume(const Maths::Vector3& halfDims)
            : m_HalfSizes(halfDims) {
            Type = VolumeType::OBB;
        }

        ~OBBVolume() = default;

        Maths::Vector3 GetHalfDimensions() const {
            return m_HalfSizes;
        }

    protected:
        Maths::Vector3 m_HalfSizes;
    };
}
