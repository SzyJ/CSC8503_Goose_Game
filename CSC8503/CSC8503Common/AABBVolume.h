#pragma once

#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
#include "Debug.h"

namespace NCL {

    class AABBVolume : CollisionVolume {
    public:
        AABBVolume(const Vector3& halfDims)
            : m_HalfSizes(halfDims) {
            Type = VolumeType::AABB;
        }
        ~AABBVolume() = default;

        void DrawDebug(const Vector3& position, const Vector4& colour) {
                // draw the cube's bounding lines
                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y + m_HalfSizes.y, position.z + m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y + m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y - m_HalfSizes.y, position.z + m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y - m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y - m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y - m_HalfSizes.y, position.z - m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y + m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y + m_HalfSizes.y, position.z - m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x + m_HalfSizes.x, position.y - m_HalfSizes.y, position.z + m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y + m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x + m_HalfSizes.x, position.y - m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y + m_HalfSizes.y, position.z - m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y - m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x - m_HalfSizes.x, position.y + m_HalfSizes.y, position.z - m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y - m_HalfSizes.y, position.z + m_HalfSizes.z),
                    Vector3(position.x - m_HalfSizes.x, position.y + m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x + m_HalfSizes.x, position.y + m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y + m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x + m_HalfSizes.x, position.y - m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x + m_HalfSizes.x, position.y - m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y - m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x - m_HalfSizes.x, position.y - m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );

                Debug::DrawLine(
                    Vector3(position.x - m_HalfSizes.x, position.y + m_HalfSizes.y, position.z - m_HalfSizes.z),
                    Vector3(position.x - m_HalfSizes.x, position.y + m_HalfSizes.y, position.z + m_HalfSizes.z),
                    colour
                );
        }

        Vector3 GetHalfDimensions() const {
            return m_HalfSizes;
        }

    protected:
        Vector3 m_HalfSizes;
    };

}
