#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "../../Common/Vector2.h"
#include "../../Common/Quaternion.h"
#include "../../Common/Window.h"
#include "../../Common/Maths.h"

#include <list>

#include "../CSC8503Common/Simplex.h"

#include "Debug.h"

using namespace NCL;

const Vector3 CollisionDetection::s_BoxFaces[6] = {
    Vector3(-1, 0, 0), Vector3(1, 0, 0),
    Vector3(0, -1, 0), Vector3(0, 1, 0),
    Vector3(0, 0, -1), Vector3(0, 0, 1),
};

bool CollisionDetection::RayPlaneIntersection(const Ray& r, const Plane& p, RayCollision& collisions) {
    return false;
}

bool CollisionDetection::RayIntersection(const Ray& r, GameObject& object, RayCollision& collision) {
    const Transform& transform = object.GetConstTransform();
    const CollisionVolume* volume = object.GetBoundingVolume();

    if (!volume) {
        return false;
    }

    switch(volume->Type) {
    case VolumeType::AABB:
        return RayAABBIntersection(r, transform, (const AABBVolume&) *volume, collision);
    case VolumeType::OBB:
        return RayOBBIntersection(r, transform, (const OBBVolume&) *volume, collision);
    case VolumeType::Sphere:
        return RaySphereIntersection(r, transform, (const SphereVolume&) *volume, collision);
    default:
        break;
    }

    return false;
}

bool CollisionDetection::RayBoxIntersection(const Ray& r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision) {
    Vector3 boxMin = boxPos - boxSize;
    Vector3 boxMax = boxPos + boxSize;

    Vector3 rayPosition = r.GetPosition();
    Vector3 rayDirection = r.GetDirection();

    Vector3 intVals(-1.0f, -1.0f, -1.0f);

    for (int vecPos = 0; vecPos < 3; ++vecPos) {
        if (rayDirection[vecPos] > 0.0f) {
            intVals[vecPos] = (boxMin[vecPos] - rayPosition[vecPos]) / rayDirection[vecPos];
        } else if (rayDirection[vecPos] < 0.0f) {
            intVals[vecPos] = (boxMax[vecPos] - rayPosition[vecPos]) / rayDirection[vecPos];
        }
    }

    float distanceToPoint = intVals.GetMaxElement();

    if (distanceToPoint < 0.0f) {
        return false;
    }

    Vector3 intersection = rayPosition + (rayDirection * distanceToPoint);

    for (int vecPos = 0; vecPos < 3; ++vecPos) {
        if (intersection[vecPos] + FLT_EPSILON < boxMin[vecPos] ||
            intersection[vecPos] - FLT_EPSILON > boxMax[vecPos]) {
            return false;
        }
    }

    collision.CollidedAt = intersection;
    collision.RayDistance = distanceToPoint;

    return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray& r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision) {
    return RayBoxIntersection(r, worldTransform.GetWorldPosition(), volume.GetHalfDimensions(), collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray& r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) {
    Quaternion orientation = worldTransform.GetLocalOrientation();
    Vector3 position = worldTransform.GetWorldPosition();

    Matrix3 invTransform = orientation.Conjugate().ToMatrix3();

    Vector3 localRayPosition = r.GetPosition() - position;

    Ray transformedRay(invTransform * localRayPosition, invTransform * r.GetDirection());

    bool collided = RayBoxIntersection(transformedRay, Vector3(), volume.GetHalfDimensions(), collision);

    if (collided) {
        Matrix3 transform = orientation.ToMatrix3();
        collision.CollidedAt = transform * collision.CollidedAt + position;
    }

    return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray& r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision) {
    Vector3 spherePosition = worldTransform.GetWorldPosition();
    float sphereRadius = volume.GetRadius();

    Vector3 rayDirection = r.GetDirection();
    Vector3 rayPosition = r.GetPosition();

    Vector3 direction = spherePosition - rayPosition;
    float sphereProjection = direction.Dot(rayDirection);
    Vector3 closestPoint = rayPosition + (rayDirection * sphereProjection);
    float sphereDistance = (closestPoint - spherePosition).Length();

    if (sphereDistance > sphereRadius) {
        return false;
    }

    collision.RayDistance = (closestPoint - rayPosition).Length();
    collision.CollidedAt = rayPosition + (rayDirection * collision.RayDistance);

    return true;
}

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
    const CollisionVolume* aVolume = a->GetBoundingVolume();
    const CollisionVolume* bVolume = b->GetBoundingVolume();

    if (!aVolume || !bVolume) {
        return false;
    }

    if (a->IsSleeping() && b->IsSleeping()) {
        return false;
    }

    collisionInfo.A = a;
    collisionInfo.B = b;

    const Transform& aTransform = a->GetConstTransform();
    const Transform& bTransform = b->GetConstTransform();

    unsigned int pairType = aVolume->TypeAsInt() | bVolume->TypeAsInt();

    if (pairType & static_cast<unsigned int>(VolumeType::OBB) &&
        pairType & static_cast<unsigned int>(VolumeType::Sphere)) {
        if (aVolume->Type == VolumeType::OBB &&
            bVolume->Type == VolumeType::Sphere) {
            return OBBSphereIntersection((OBBVolume&)*aVolume, aTransform, (SphereVolume&)*bVolume, bTransform, collisionInfo);
        } else if (
            aVolume->Type == VolumeType::Sphere &&
            bVolume->Type == VolumeType::OBB) {
            collisionInfo.A = b;
            collisionInfo.B = a;
            return OBBSphereIntersection((OBBVolume&)*bVolume, bTransform, (SphereVolume&)*aVolume, aTransform, collisionInfo);
        }
    }

    if (pairType & static_cast<unsigned int>(VolumeType::AABB) &&
        pairType & static_cast<unsigned int>(VolumeType::Sphere)) {
        if (aVolume->Type == VolumeType::AABB &&
            bVolume->Type == VolumeType::Sphere) {
            return AABBSphereIntersection((AABBVolume&) *aVolume, aTransform, (SphereVolume&) *bVolume, bTransform, collisionInfo);
        } else if (
            aVolume->Type == VolumeType::Sphere &&
            bVolume->Type == VolumeType::AABB) {
            collisionInfo.A = b;
            collisionInfo.B = a;
            return AABBSphereIntersection((AABBVolume&)*bVolume, bTransform, (SphereVolume&)*aVolume, aTransform, collisionInfo);
        }
    }

    if (pairType & static_cast<unsigned int>(VolumeType::AABB)) {
        return AABBIntersection((AABBVolume&)(*aVolume), aTransform, (AABBVolume&)(*bVolume), bTransform, collisionInfo);
    }

    if (pairType & static_cast<unsigned int>(VolumeType::Sphere)) {
        return SphereIntersection((SphereVolume&)(*aVolume), aTransform, (SphereVolume&)(*bVolume), bTransform, collisionInfo);
    }

    return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
    Vector3 positionDelta = posB - posA;
    Vector3 sumSize = halfSizeA + halfSizeB;
    
    return
        abs(positionDelta.x) < sumSize.x &&
        abs(positionDelta.y) < sumSize.y &&
        abs(positionDelta.z) < sumSize.z;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(
    const AABBVolume& volumeA, const Transform& worldTransformA,
    const AABBVolume& volumeB, const Transform& worldTransformB,
    CollisionInfo& collisionInfo) {

    Vector3 aPos = worldTransformA.GetWorldPosition();
    Vector3 bPos = worldTransformB.GetWorldPosition();
    Vector3 aSize = volumeA.GetHalfDimensions();
    Vector3 bSize = volumeB.GetHalfDimensions();

    bool overlap = AABBTest(aPos, bPos, aSize, bSize);

    if (!overlap) {
        return false;
    }

    const Vector3 aMax = aPos + aSize;
    const Vector3 aMin = aPos - aSize;

    const Vector3 bMax = bPos + bSize;
    const Vector3 bMin = bPos - bSize;

    float distances[6] = {
        (bMax.x - aMin.x) ,// distance of box ’b’ to ’left’ of ’a’.
        (aMax.x - bMin.x) ,// distance of box ’b’ to ’right’ of ’a’.
        (bMax.y - aMin.y) ,// distance of box ’b’ to ’bottom’ of ’a’.
        (aMax.y - bMin.y) ,// distance of box ’b’ to ’top’ of ’a’.
        (bMax.z - aMin.z) ,// distance of box ’b’ to ’far’ of ’a’.
        (aMax.z - bMin.z)  // distance of box ’b’ to ’near’ of ’a’.
    };

    float penetration = distances[0];
    Vector3 bestAxis = s_BoxFaces[0];

    for (int i = 1; i < 6; ++i) {
        if (distances[i] < penetration) {
            penetration = distances[i];
            bestAxis = s_BoxFaces[i];
        }
    }

    collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);

    return true;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(
    const SphereVolume& aVolume, const Transform& worldTransformA,
    const SphereVolume& bVolume, const Transform& worldTransformB,
    CollisionInfo& collisionInfo) {

    const float radiusSum = aVolume.GetRadius() + bVolume.GetRadius();
    const Vector3 distanceDelta = worldTransformA.GetWorldPosition() - worldTransformB.GetWorldPosition();
    const float distance = distanceDelta.Length();

    if (distance > radiusSum) {
        return false;
    }

    const float penetration = radiusSum - distance;
    const Vector3 normal = distanceDelta.Normalised();
    const Vector3 localA = -normal * aVolume.GetRadius();
    const Vector3 localB =  normal * bVolume.GetRadius();

    collisionInfo.AddContactPoint(localA, localB, -normal, penetration);

    return true;
}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(
    const AABBVolume& volumeA, const Transform& worldTransformA,
    const SphereVolume& volumeB, const Transform& worldTransformB,
    CollisionInfo& collisionInfo) {

    const Vector3 boxSize = volumeA.GetHalfDimensions();
    const Vector3 distanceDelta = worldTransformB.GetWorldPosition() - worldTransformA.GetWorldPosition();
    const Vector3 closestPointOnBox = Maths::Clamp(distanceDelta, -boxSize, boxSize);

    const Vector3 localPoint = distanceDelta - closestPointOnBox;
    const float distance = localPoint.Length();

    const float sphereRadius = volumeB.GetRadius();

    if (distance > sphereRadius) {
        return false;
    }

    const Vector3 collisionNormal = localPoint.Normalised();
    const float penetration = sphereRadius - distance;

    const Vector3 localA = closestPointOnBox;
    const Vector3 localB = -collisionNormal * sphereRadius;

    collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);

    return true;
}


bool CollisionDetection::AABBOBBIntresection(
    const AABBVolume& volumeA, const Transform& worldTransformA,
    const OBBVolume& volumeB, const Transform& worldTransformB,
    CollisionInfo& collisionInfo) {

    // Implement SAT
    Quaternion bOrientation = worldTransformB.GetLocalOrientation();
    Vector3 bPosition = worldTransformB.GetWorldPosition();
    Vector3 bSize = volumeB.GetHalfDimensions();


    return false;
}

bool CollisionDetection::OBBSphereIntersection(
    const OBBVolume& volumeA, const Transform& worldTransformA,
    const SphereVolume& volumeB, const Transform& worldTransformB,
    CollisionInfo& collisionInfo) {

    const Quaternion aOrientation = worldTransformA.GetLocalOrientation();

    const Vector3 localPositionOffset = -worldTransformA.GetWorldPosition();
    const Vector3 sphereLocalPosition = worldTransformB.GetWorldPosition() + localPositionOffset;
    const Vector3 bLocalSpacePosition = aOrientation.Conjugate() * sphereLocalPosition;

    Transform localSphereTransform;
    localSphereTransform.SetWorldPosition(bLocalSpacePosition);

    AABBVolume localCubeVolume(volumeA.GetHalfDimensions());
    Transform localCubeTransform;
    localCubeTransform.SetWorldPosition(Vector3(0.0f, 0.0f, 0.0f));

    //CollisionInfo localInfo;
    bool collided = AABBSphereIntersection(localCubeVolume, localCubeTransform, volumeB, localSphereTransform, collisionInfo);

    // Re-Add offsets to penetration and collision point
    collisionInfo.Point.Normal = aOrientation * collisionInfo.Point.Normal;
    collisionInfo.Point.LocalA = aOrientation * collisionInfo.Point.LocalA;
    collisionInfo.Point.LocalA += localPositionOffset;
    collisionInfo.Point.LocalB += localPositionOffset;

    return collided;
}


bool CollisionDetection::OBBIntersection(
    const OBBVolume& volumeA, const Transform& worldTransformA,
    const OBBVolume& volumeB, const Transform& worldTransformB,
    CollisionInfo& collisionInfo) {

    // Rotate and resolve as AABB and OBB.

    return false;
}

//It's helper functions for generating rays from here on out:

Matrix4 GenerateInverseView(const Camera& c) {
    float pitch = c.GetPitch();
    float yaw = c.GetYaw();
    Vector3 position = c.GetPosition();

    Matrix4 iview = Matrix4::Translation(position) *
        Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
        Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

    return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float nearPlane, float farPlane, float fov) {
    float negDepth = nearPlane - farPlane;

    float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

    Matrix4 m;

    float h = 1.0f / tan(fov * PI_OVER_360);

    m.array[0] = aspect / h;
    m.array[5] = tan(fov * PI_OVER_360);
    m.array[10] = 0.0f;

    m.array[11] = invNegDepth; //// +PI_OVER_360;
    m.array[14] = -1.0f;
    m.array[15] = (0.5f / nearPlane) + (0.5f / farPlane);

    //Matrix4 temp = projection.Inverse();
    //return temp;
    return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const Camera& cam) {
    Vector2 screenSize = Window::GetWindow()->GetScreenSize();

    float aspect = screenSize.x / screenSize.y;
    float fov = cam.GetFieldOfVision();
    float nearPlane = cam.GetNearPlane();
    float farPlane = cam.GetFarPlane();

    //Create our inverted matrix! Note how that to get a correct inverse matrix,
    //the order of matrices used to form it are inverted, too.
    Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

    Matrix4 test1 = GenerateInverseView(cam);
    Matrix4 test2 = cam.BuildViewMatrix().Inverse();

    Matrix4 proj = cam.BuildProjectionMatrix(aspect);
    Matrix4 test4 = cam.BuildProjectionMatrix(aspect).Inverse();
    Matrix4 test3 = GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

    //Our mouse position x and y values are in 0 to screen dimensions range,
    //so we need to turn them into the -1 to 1 axis range of clip space.
    //We can do that by dividing the mouse values by the width and height of the
    //screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
    //and then subtracting 1 (-1.0 to 1.0).
    Vector4 clipSpace = Vector4(
        (screenPos.x / (float) screenSize.x) * 2.0f - 1.0f,
        (screenPos.y / (float) screenSize.y) * 2.0f - 1.0f,
        (screenPos.z),
        1.0f
    );

    //Then, we multiply our clipspace coordinate by our inverted matrix
    Vector4 transformed = invVP * clipSpace;

    //our transformed w coordinate is now the 'inverse' perspective divide, so
    //we can reconstruct the final world space by dividing x,y,and z by w.
    return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const Camera& cam) {
    Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
    Vector2 screenSize = Window::GetWindow()->GetScreenSize();

    //We remove the y axis mouse position from height as OpenGL is 'upside down',
    //and thinks the bottom left is the origin, instead of the top left!
    Vector3 nearPos = Vector3(screenMouse.x,
                              screenSize.y - screenMouse.y,
                              -0.99999f
    );

    //We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
    //causes the unproject function to go a bit weird. 
    Vector3 farPos = Vector3(screenMouse.x,
                             screenSize.y - screenMouse.y,
                             0.99999f
    );

    Vector3 a = Unproject(nearPos, cam);
    Vector3 b = Unproject(farPos, cam);
    Vector3 c = b - a;

    c.Normalise();

    //std::cout << "Ray Direction:" << c << std::endl;

    return Ray(cam.GetPosition(), c);
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
    Matrix4 m;

    float t = tan(fov * PI_OVER_360);

    float neg_depth = nearPlane - farPlane;

    const float h = 1.0f / t;

    float c = (farPlane + nearPlane) / neg_depth;
    float e = -1.0f;
    float d = 2.0f * (nearPlane * farPlane) / neg_depth;

    m.array[0] = aspect / h;
    m.array[5] = tan(fov * PI_OVER_360);

    m.array[10] = 0.0f;
    m.array[11] = 1.0f / d;

    m.array[14] = 1.0f / e;

    m.array[15] = -c / (d * e);

    return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera& c) {
    float pitch = c.GetPitch();
    float yaw = c.GetYaw();
    Vector3 position = c.GetPosition();

    Matrix4 iview = Matrix4::Translation(position) *
        Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
        Matrix4::Rotation(pitch, Vector3(1, 0, 0));

    return iview;
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3 CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const Camera& c) {
    //Create our inverted matrix! Note how that to get a correct inverse matrix,
    //the order of matrices used to form it are inverted, too.
    Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());

    Vector2 screenSize = Window::GetWindow()->GetScreenSize();

    //Our mouse position x and y values are in 0 to screen dimensions range,
    //so we need to turn them into the -1 to 1 axis range of clip space.
    //We can do that by dividing the mouse values by the width and height of the
    //screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
    //and then subtracting 1 (-1.0 to 1.0).
    Vector4 clipSpace = Vector4(
        (position.x / (float) screenSize.x) * 2.0f - 1.0f,
        (position.y / (float) screenSize.y) * 2.0f - 1.0f,
        (position.z) - 1.0f,
        1.0f
    );

    //Then, we multiply our clipspace coordinate by our inverted matrix
    Vector4 transformed = invVP * clipSpace;

    //our transformed w coordinate is now the 'inverse' perspective divide, so
    //we can reconstruct the final world space by dividing x,y,and z by w.
    return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}
