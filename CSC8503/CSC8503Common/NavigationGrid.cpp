#include "NavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE = 0;
const int RIGHT_NODE = 1;
const int TOP_NODE = 2;
const int BOTTOM_NODE = 3;
const char WALL_NODE = 'x';
const char FLOOR_NODE = '.';

NavigationGrid::NavigationGrid() {
    m_NodeSize = 0;
    m_GridWidth = 0;
    m_GridHeight = 0;
    m_AllNodes = nullptr;
}

NavigationGrid::NavigationGrid(const std::string& filename) : NavigationGrid() {
    std::ifstream infile(Assets::DATADIR + filename);

    infile >> m_NodeSize;
    infile >> m_GridWidth;
    infile >> m_GridHeight;

    m_AllNodes = new GridNode[m_GridWidth * m_GridHeight];

    for (int y = 0; y < m_GridHeight; ++y) {
        for (int x = 0; x < m_GridWidth; ++x) {
            GridNode& n = m_AllNodes[(m_GridWidth * y) + x];
            char type = 0;
            infile >> type;
            n.Type = type;
            n.Position = Vector3((float) (x * m_GridWidth), 0, (float) (y * m_GridHeight));
        }
    }

    //now to build the connectivity between the nodes
    for (int y = 0; y < m_GridHeight; ++y) {
        for (int x = 0; x < m_GridWidth; ++x) {
            GridNode& n = m_AllNodes[(m_GridWidth * y) + x];

            if (y > 0) {
                //get the above node
                n.Connected[0] = &m_AllNodes[(m_GridWidth * (y - 1)) + x];
            }
            if (y < m_GridHeight - 1) {
                //get the below node
                n.Connected[1] = &m_AllNodes[(m_GridWidth * (y + 1)) + x];
            }
            if (x > 0) {
                //get left node
                n.Connected[2] = &m_AllNodes[(m_GridWidth * (y)) + (x - 1)];
            }
            if (x < m_GridWidth - 1) {
                //get right node
                n.Connected[3] = &m_AllNodes[(m_GridWidth * (y)) + (x + 1)];
            }
            for (int i = 0; i < 4; ++i) {
                if (n.Connected[i]) {
                    if (n.Connected[i]->Type == '.') {
                        n.Costs[i] = 1;
                    }
                    if (n.Connected[i]->Type == 'x') {
                        n.Connected[i] = nullptr; //actually a wall, disconnect!
                    }
                }
            }
        }
    }
}

NavigationGrid::~NavigationGrid() {
    delete[] m_AllNodes;
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
    return false; //open list emptied out with no path!
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
    return false;
}

GridNode* NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
    return nullptr;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
    return 0.0f;
}
