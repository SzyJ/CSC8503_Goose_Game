#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "../../Common/Camera.h"
#include <algorithm>

using namespace NCL;
using namespace NCL::CSC8503;

void GameWorld::Clear() {
    m_GameObjects.clear();
    m_Constraints.clear();
}

void GameWorld::ClearAndErase() {
    for (auto& i : m_GameObjects) {
        delete i;
    }
    for (auto& i : m_Constraints) {
        delete i;
    }
    Clear();
}

void GameWorld::AddGameObject(GameObject* o) {
    m_GameObjects.emplace_back(o);
}

void GameWorld::RemoveGameObject(GameObject* o) {
    std::remove(m_GameObjects.begin(), m_GameObjects.end(), o);
}

void GameWorld::GetObjectIterators(
    GameObjectIterator& first,
    GameObjectIterator& last) const {

    first = m_GameObjects.begin();
    last = m_GameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
    for (GameObject* g : m_GameObjects) {
        f(g);
    }
}

void GameWorld::UpdateWorld(float dt) {
    UpdateTransforms();

    if (m_ShuffleObjects) {
        std::random_shuffle(m_GameObjects.begin(), m_GameObjects.end());
    }

    if (m_ShuffleConstraints) {
        std::random_shuffle(m_Constraints.begin(), m_Constraints.end());
    }
}

void GameWorld::UpdateTransforms() {
    for (auto& i : m_GameObjects) {
        i->GetTransform().UpdateMatrices();
    }
}

void GameWorld::UpdateQuadTree() {
    delete m_QuadTree;

    //quadTree = new QuadTree<GameObject*>(Vector2(512, 512), 6);

    //for (auto& i : gameObjects) {
    //    quadTree->Insert(i);
    //}
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject) const {
    //The simplest raycast just goes through each object and sees if there's a collision
    RayCollision collision;

    for (auto& i : m_GameObjects) {
        if (!i->GetBoundingVolume()) {
            //objects might not be collideable etc...
            continue;
        }
        RayCollision thisCollision;
        if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {

            if (!closestObject) {
                closestCollision = collision;
                closestCollision.Node = i;
                return true;
            } else {
                if (thisCollision.RayDistance < collision.RayDistance) {
                    thisCollision.Node = i;
                    collision = thisCollision;
                }
            }
        }
    }
    if (collision.Node) {
        closestCollision = collision;
        closestCollision.Node = collision.Node;
        return true;
    }
    return false;
}


/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint* c) {
    m_Constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c) {
    std::remove(m_Constraints.begin(), m_Constraints.end(), c);
}

void GameWorld::GetConstraintIterators(
    std::vector<Constraint*>::const_iterator& first,
    std::vector<Constraint*>::const_iterator& last) const {
    first = m_Constraints.begin();
    last = m_Constraints.end();
}
