#pragma once

namespace NCL {

    enum class VolumeType {
        AABB = 1,
        OBB = 2,
        Sphere = 4,
        Mesh = 8,
        Compound= 16,
        Invalid = 256
    };

    class CollisionVolume {
    public:
        CollisionVolume()
            : m_Type(VolumeType::Invalid) { }

        ~CollisionVolume() = default;

        VolumeType m_Type;
    };

}
