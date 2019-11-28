#pragma once

#include <vector>
#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"

namespace NCL {

    class Camera;
    using Maths::Ray;

    namespace CSC8503 {

        class GameObject;
        class Constraint;
        typedef std::function<void(GameObject*)> GameObjectFunc;
        typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

        class GameWorld {
        public:
            GameWorld()
                :m_QuadTree(nullptr), m_ShuffleConstraints(false), m_ShuffleObjects(false) {
                m_MainCamera = new Camera();
            }

            ~GameWorld() = default;

            void Clear();
            void ClearAndErase();

            void AddGameObject(GameObject* o);
            void RemoveGameObject(GameObject* o);

            void AddConstraint(Constraint* c);
            void RemoveConstraint(Constraint* c);

            Camera* GetMainCamera() const {
                return m_MainCamera;
            }

            void ShuffleConstraints(bool state) {
                m_ShuffleConstraints = state;
            }

            void ShuffleObjects(bool state) {
                m_ShuffleObjects = state;
            }

            bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

            virtual void UpdateWorld(float dt);

            void OperateOnContents(GameObjectFunc f);

            void GetObjectIterators(
                GameObjectIterator& first,
                GameObjectIterator& last) const;

            void GetConstraintIterators(
                std::vector<Constraint*>::const_iterator& first,
                std::vector<Constraint*>::const_iterator& last) const;

        protected:
            void UpdateTransforms();
            void UpdateQuadTree();

            std::vector<GameObject*> m_GameObjects;
            std::vector<Constraint*> m_Constraints;
            QuadTree<GameObject*>* m_QuadTree;
            Camera* m_MainCamera;
            bool m_ShuffleConstraints;
            bool m_ShuffleObjects;
        };

    }
}
