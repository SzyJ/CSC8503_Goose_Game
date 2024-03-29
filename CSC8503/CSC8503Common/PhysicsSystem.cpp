#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "../../Common/Quaternion.h"

#include "Constraint.h"

#include "Debug.h"

#include <functional>

using namespace NCL;
using namespace CSC8503;

void PhysicsSystem::SetGravity(const Vector3& g) {
    m_Gravity = g;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
    m_AllCollisions.clear();
}

/*

This is the core of the physics engine update

*/
void PhysicsSystem::Update(float dt) {
    //GameTimer testTimer;

    m_FrameDT = dt;

    m_DTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

    float iterationDt = 1.0f / 120.0f; //Ideally we'll have 120 physics updates a second 

    if (m_DTOffset > 8 * iterationDt) {
        //the physics engine cant catch up!
        iterationDt = 1.0f / 15.0f; //it'll just have to run bigger timesteps...
        //std::cout << "Setting physics iterations to 15" << iterationDt << std::endl;
    } else if (m_DTOffset > 4 * iterationDt) {
        //the physics engine cant catch up!
        iterationDt = 1.0f / 30.0f; //it'll just have to run bigger timesteps...
        //std::cout << "Setting iteration dt to 4 case " << iterationDt << std::endl;
    } else if (m_DTOffset > 2 * iterationDt) {
        //the physics engine cant catch up!
        iterationDt = 1.0f / 60.0f; //it'll just have to run bigger timesteps...
        //std::cout << "Setting iteration dt to 2 case " << iterationDt << std::endl;
    } else {
        //std::cout << "Running normal update " << iterationDt << std::endl;
    }

    int constraintIterationCount = 10;
    //iterationDt = dt;

    if (m_UseBroadPhase) {
        UpdateObjectAABBs();
    }

    while (m_DTOffset > iterationDt * 0.5) {
        IntegrateAccel(iterationDt); //Update accelerations from external forces
        if (m_UseBroadPhase) {
            BroadPhase();
            NarrowPhase();
        } else {
            BasicCollisionDetection();
        }

        //This is our simple iterative solver - 
        //we just run things multiple times, slowly moving things forward
        //and then rechecking that the constraints have been met        
        float constraintDt = iterationDt / (float) constraintIterationCount;

        for (int i = 0; i < constraintIterationCount; ++i) {
            UpdateConstraints(constraintDt);
        }

        IntegrateVelocity(iterationDt); //update positions from new velocity changes

        m_DTOffset -= iterationDt;
    }
    ClearForces(); //Once we've finished with the forces, reset them to zero

    UpdateCollisionList(); //Remove any old collisions
    //std::cout << iteratorCount << " , " << iterationDt << std::endl;
    //float time = testTimer.GetTimeDeltaSeconds();
    //std::cout << "Physics time taken: " << time << std::endl;
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
    for (auto i = m_AllCollisions.begin(); i != m_AllCollisions.end();) {
        if ((*i).FramesLeft == m_NumCollisionFrames) {
            i->A->OnCollisionBegin(i->B);
            i->B->OnCollisionBegin(i->A);
        }

        (*i).FramesLeft = (*i).FramesLeft - 1;
        if ((*i).FramesLeft < 0) {
            i->A->OnCollisionEnd(i->B);
            i->B->OnCollisionEnd(i->A);
            i = m_AllCollisions.erase(i);
        } else {
            ++i;
        }
    }
}

void PhysicsSystem::UpdateObjectAABBs() {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i) {
        (*i)->UpdateBroadphaseAABB();
    }
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    for (auto aObj = first; aObj != last; ++aObj) {
        if ((*aObj)->GetPhysicsObject() == nullptr) {
            continue;
        }

        for (auto bObj = aObj + 1; bObj != last; ++bObj) {
            if ((*bObj)->GetPhysicsObject() == nullptr) {
                continue;
            }

            if (!(*aObj)->IsActive() && !(*bObj)->IsActive()) {
                continue;
            }

            if ((*aObj)->IsSleeping() && (*bObj)->IsSleeping()) {
                continue;
            }

            CollisionDetection::CollisionInfo info;
            if (CollisionDetection::ObjectIntersection(*aObj, *bObj, info)) {
                //std::cout << " Collision between " << (*aObj)->GetName() << " and " << (*bObj)->GetName() << std::endl;
                ImpulseResolveCollision(*info.A, *info.B, info.Point);
                info.FramesLeft = m_NumCollisionFrames;
                m_AllCollisions.insert(info);
            }

        }
    }
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {

    if (a.GetName() == "Water" ||
        b.GetName() == "Water") {
        return;
    }

    PhysicsObject* aPhysObj = a.GetPhysicsObject();
    PhysicsObject* bPhysObj = b.GetPhysicsObject();

    Transform & aTransform = a.GetTransform();
    Transform & bTransform = b.GetTransform();

    const float aInvMass = aPhysObj->GetInverseMass();
    const float bInvMass = bPhysObj->GetInverseMass();
    
    const Vector3 aPosition = aTransform.GetWorldPosition();
    const Vector3 bPosition = bTransform.GetWorldPosition();
    
    const float massSum = aInvMass + bInvMass;
    
    const Vector3 aResolve = p.Normal * p.Penetration * (aInvMass / massSum);
    const Vector3 bResolve = p.Normal * p.Penetration * (bInvMass / massSum);
    
    aTransform.SetWorldPosition(aPosition - aResolve);
    bTransform.SetWorldPosition(bPosition + bResolve);
    
    const Vector3 aRelPos = p.LocalA;
    const Vector3 bRelPos = p.LocalB;
    
    const Vector3 aAngVel = aPhysObj->GetAngularVelocity().Cross(aRelPos);
    const Vector3 bAngVel = bPhysObj->GetAngularVelocity().Cross(bRelPos);
    
    const Vector3 aFullVel = aPhysObj->GetLinearVelocity() + aAngVel;
    const Vector3 bFullVel = bPhysObj->GetLinearVelocity() + bAngVel;
    
    const Vector3 constantVel = bFullVel - aFullVel;
    
    const float impulseForce = Vector3::Dot(constantVel, p.Normal);
    
    const Vector3 aInertia = Vector3::Cross(
        aPhysObj->GetInertiaTensor() * aRelPos.Cross(p.Normal),
        aRelPos);
    const Vector3 bInertia = Vector3::Cross(
        bPhysObj->GetInertiaTensor() * bRelPos.Cross(p.Normal),
        bRelPos);
    
    const float angularEffect = Vector3::Dot(aInertia + bInertia, p.Normal);
    const float cRestitution = 0.66f;

    const float j = (-impulseForce * (1.0f + cRestitution)) / (massSum + angularEffect);
    
    const Vector3 fullImpulse = p.Normal * j;


    if (!strcmp(a.GetName().c_str(), "Goose") &&
        !strcmp(b.GetName().c_str(), "Apple")) {

        bPhysObj->ApplyLinearImpulse(fullImpulse);
        bPhysObj->ApplyAngularImpulse(bRelPos.Cross(fullImpulse));

    } else if (!strcmp(a.GetName().c_str(), "Apple") &&
        !strcmp(b.GetName().c_str(), "Goose")) {

        aPhysObj->ApplyLinearImpulse(-fullImpulse);
        aPhysObj->ApplyAngularImpulse(aRelPos.Cross(-fullImpulse));

    } else {
        aPhysObj->ApplyLinearImpulse(-fullImpulse);
        bPhysObj->ApplyLinearImpulse(fullImpulse);
    
        aPhysObj->ApplyAngularImpulse(aRelPos.Cross(-fullImpulse));
        bPhysObj->ApplyAngularImpulse(bRelPos.Cross(fullImpulse));
    }
    
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/

void PhysicsSystem::BroadPhase() {
    m_BroadphaseCollisions.clear();

    QuadTree<GameObject*> tree(Vector2(1024, 1024), 7, 6);

    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    for (auto obj = first; obj != last; ++obj) {
        Vector3 halfSize;
        if (!(*obj)->GetBroadphaseAABB(halfSize)) {
            continue;
        }

        Vector3 pos = (*obj)->GetConstTransform().GetWorldPosition();
        tree.Insert(*obj, pos, halfSize);
    }

    tree.OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data) {
    CollisionDetection::CollisionInfo info;
        for (auto i = data.begin(); i != data.end(); ++i) {
            for (auto j = std::next(i); j != data.end(); ++j) {
                info.A = min(i->Object, j->Object);
                info.B = max(i->Object, j->Object);
                m_BroadphaseCollisions.insert(info);
            }
        }
    });
}

/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
    for (const auto& info : m_BroadphaseCollisions) {
        auto infoCopy = info;
        if (CollisionDetection::ObjectIntersection(infoCopy.A, infoCopy.B, infoCopy)) {
            infoCopy.FramesLeft = m_NumCollisionFrames;
            ImpulseResolveCollision(*infoCopy.A, *infoCopy.B, infoCopy.Point);
            m_AllCollisions.insert(infoCopy);
        }
    }
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    for (auto obj = first; obj < last; ++obj) {
        if ((*obj)->IsSleeping()) {
            continue;
        }

        PhysicsObject* physObject = (*obj)->GetPhysicsObject();

        if (!physObject) {
            continue;
        }

        float invMass = physObject->GetInverseMass();

        Vector3 linearVel = physObject->GetLinearVelocity();
        Vector3 force = physObject->GetForce();
        Vector3 accel = force * invMass;

        if (m_ApplyGravity && invMass > 0) {

            Vector3 gravAccel = m_Gravity;
        
            accel += physObject->GetGravityDirection() * 9.81f;
        }

        linearVel += accel * dt;

        physObject->SetLinearVelocity(linearVel);

        // Angular movement
        Vector3 torque = physObject->GetTorque();
        Vector3 angularVel = physObject->GetAngularVelocity();

        physObject->UpdateInertiaTensor();

        Vector3 angularAccel = physObject->GetInertiaTensor() * torque;
        angularVel += angularAccel * dt;

        physObject->SetAngularVelocity(angularVel);
    }
}

/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    float dampeningFactor = 1.0f - 0.95f;
    float frameDampening = powf(dampeningFactor, dt);

    for (auto obj = first; obj < last; ++obj) {
        if ((*obj)->IsSleeping()) {
            continue;
        }

        PhysicsObject* physObject = (*obj)->GetPhysicsObject();
        if (!physObject) {
            continue;
        }
        Transform& transform = (*obj)->GetTransform();

        Vector3 position = transform.GetLocalPosition();
        Vector3 linearVel = physObject->GetLinearVelocity();
        Vector3 positionDelta = linearVel * dt;

        position += positionDelta;
        (*obj)->AddPositionDelta(positionDelta);

        transform.SetLocalPosition(position);

        linearVel *= frameDampening;
        physObject->SetLinearVelocity(linearVel);

        // Orientation
        Quaternion orientation = transform.GetLocalOrientation();
        Vector3 angularVel = physObject->GetAngularVelocity();

        orientation += Quaternion(angularVel * dt * 0.5f, 0.0f) * orientation;
        orientation.Normalise();

        transform.SetLocalOrientation(orientation);
        angularVel *= frameDampening;

        physObject->SetAngularVelocity(angularVel);
    }
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i) {
        //Clear our object's forces for the next frame
        (*i)->GetPhysicsObject()->ClearForces();
    }
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
    std::vector<Constraint*>::const_iterator first;
    std::vector<Constraint*>::const_iterator last;
    m_GameWorld.GetConstraintIterators(first, last);

    for (auto i = first; i != last; ++i) {
        (*i)->UpdateConstraint(dt);
    }
}


void PhysicsSystem::UpdateDebugColours() {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_GameWorld.GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i) {
        if ((*i)->IsSleeping()) {
            (*i)->GetRenderObject()->SetColour(Vector3(1.0, 0.0f, 0.0f));
        } else {
            (*i)->GetRenderObject()->SetColour(Vector3(1.0, 1.0f, 1.0f));
        }
    }
}
