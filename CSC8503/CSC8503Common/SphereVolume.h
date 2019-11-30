#pragma once

#include "CollisionVolume.h"

namespace NCL {

    class SphereVolume : CollisionVolume {
    public:
        SphereVolume(float sphereRadius = 1.0f)
            : m_Radius(sphereRadius) {
            Type = VolumeType::Sphere;
        }
        ~SphereVolume() = default;

        float GetRadius() const {
            return m_Radius;
        }

    protected:
        float m_Radius;
    };

}
