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

    struct CollisionVolume {
        VolumeType Type;

        CollisionVolume()
            : Type(VolumeType::Invalid) { }
        CollisionVolume(VolumeType type)
            : Type(type) { }
        ~CollisionVolume() = default;

        unsigned int TypeAsInt() const { return static_cast<unsigned int>(Type); }
    };

}
