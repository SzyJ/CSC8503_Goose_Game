#pragma once

#include "BoundingVolume.h"

namespace NCL {

    class BoundingOOBB : BoundingVolume {
    public:
        BoundingOOBB() {
            m_Type = BoundingType::OOBB;
        }
        ~BoundingOOBB() = default;

        Vector3 m_Origin;
        Vector3 m_HalfSizes;
    };

}
