#pragma once

#include "../../Common/Vector3.h"

using namespace NCL::Maths;

namespace NCL {

    enum class BoundingType {
        AABB,
        OOBB,
        Sphere,
        Mesh
    };

    class BoundingVolume {
    public:
        BoundingVolume() = default;
        ~BoundingVolume() = default;

        BoundingType m_Type;
    };

}
