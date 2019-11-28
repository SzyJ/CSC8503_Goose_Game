#pragma once

#include "NavigationMap.h"
#include <string>

namespace NCL {
    namespace CSC8503 {

        struct GridNode {
            GridNode* Parent;
            GridNode* Connected[4];
            int Costs[4];
            Vector3 Position;
            float F;
            float G;
            int Type;

            GridNode() {
                for (int i = 0; i < 4; ++i) {
                    Connected[i] = nullptr;
                    Costs[i] = 0;
                }
                F = 0;
                G = 0;
                Type = 0;
                Parent = nullptr;
            }

            ~GridNode() = default;
        };

        class NavigationGrid : public NavigationMap {
        public:
            NavigationGrid();
            NavigationGrid(const std::string& filename);
            ~NavigationGrid();

            bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;

        protected:
            bool NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
            GridNode* RemoveBestNode(std::vector<GridNode*>& list) const;
            float Heuristic(GridNode* hNode, GridNode* endNode) const;

            int m_NodeSize;
            int m_GridWidth;
            int m_GridHeight;
            GridNode* m_AllNodes;
        };

    }
}
