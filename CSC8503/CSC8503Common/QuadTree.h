#pragma once

#include "../../Common/Vector2.h"
#include "Debug.h"
#include <list>
#include <functional>

namespace NCL {

    using namespace NCL::Maths;

    namespace CSC8503 {

        template <class T>
        class QuadTree;

        template <class T>
        struct QuadTreeEntry {
            Vector3 Pos;
            Vector3 Size;
            T Object;

            QuadTreeEntry(T obj, Vector3 pos, Vector3 size)
                : Object(obj), Pos(pos), Size(size) { }
        };

        template <class T>
        class QuadTreeNode {
        public:
            typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;

        protected:
            friend class QuadTree<T>;

            QuadTreeNode() = default;
            QuadTreeNode(Vector2 pos, Vector2 size)
                : m_Position(pos), m_Size(size) {}
            ~QuadTreeNode() {
                delete[] m_Children;
            }

            void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) { }

            void Split() { }

            void DebugDraw() { }

            void OperateOnContents(QuadTreeFunc& func) { }

        protected:
            std::list<QuadTreeEntry<T>> m_Contents;
            Vector2 m_Position;
            Vector2 m_Size;
            QuadTreeNode<T>* m_Children = nullptr;
        };

    }
}


namespace NCL {

    using namespace NCL::Maths;

    namespace CSC8503 {

        template <class T>
        class QuadTree {
        public:
            QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5)
                : m_Root(Vector2(), size), m_MaxDepth(maxDepth), m_MaxSize(maxSize) { }
            ~QuadTree() = default;

            void Insert(T object, const Vector3& pos, const Vector3& size) {
                m_Root.Insert(object, pos, size, m_MaxDepth, m_MaxSize);
            }

            void DebugDraw() {
                m_Root.DebugDraw();
            }

            void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc func) {
                m_Root.OperateOnContents(func);
            }

        protected:
            QuadTreeNode<T> m_Root;
            int m_MaxDepth;
            int m_MaxSize;
        };

    }
}
