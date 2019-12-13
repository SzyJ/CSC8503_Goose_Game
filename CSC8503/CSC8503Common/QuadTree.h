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

            void Insert(T & object, const Vector3 & objectPos, const Vector3 & objectSize, int depthLeft, int maxSize) {
                if (!CollisionDetection::AABBTest(objectPos, Vector3(m_Position.x, 0.0f, m_Position.y), objectSize, Vector3(m_Size.x, 1000.0f, m_Size.y))) {
                    return;
                }
                if (m_Children) {
                    for (int i = 0; i < 4; ++i) {
                        m_Children[i].Insert(object, objectPos, objectSize, depthLeft, maxSize);
                    }
                    return;
                }
                
                m_Contents.emplace_back(QuadTreeEntry<T>(object, objectPos, objectSize));

                if (static_cast<int>(m_Contents.size()) > maxSize&& depthLeft > 0 && !m_Children) {
                    Split();

                    for (const auto& entry : m_Contents) {
                        for (int j = 0; j < 4; ++j) {
                            auto entryCopy = entry;
                            m_Children[j].Insert(entryCopy.Object, entryCopy.Pos, entryCopy.Size, depthLeft - 1, maxSize);
                        }
                    }

                    m_Contents.clear();
                }
            }

            void Split() {
                Vector2 halfSize = m_Size / 2.0f;
                m_Children = new QuadTreeNode<T>[4];
                m_Children[0] = QuadTreeNode<T>(m_Position + Vector2(-halfSize.x, halfSize.y), halfSize);
                m_Children[1] = QuadTreeNode<T>(m_Position + Vector2(halfSize.x, halfSize.y), halfSize);
                m_Children[2] = QuadTreeNode<T>(m_Position + Vector2(-halfSize.x, -halfSize.y), halfSize);
                m_Children[3] = QuadTreeNode<T>(m_Position + Vector2(halfSize.x, -halfSize.y), halfSize);
            }

            void DebugDraw();

            void OperateOnContents(QuadTreeFunc & func) {
                if (m_Children) {
                    for (int i = 0; i < 4; ++i) {
                        m_Children[i].OperateOnContents(func);
                    }
                } else {
                    if (!m_Contents.empty()) {
                        func(m_Contents);
                    }
                }
            }

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
