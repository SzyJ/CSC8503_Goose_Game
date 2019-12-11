#include "NavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>
#include "Debug.h"

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
    // need to work out which node ’from ’ sits in , and ’to ’ sits in
    const int fromX = (from.x / (float) m_NodeSize);
    const int fromZ = (from.z / (float) m_NodeSize);

    const int toX = (to.x / (float) m_NodeSize);
    const int toZ = (to.z / (float) m_NodeSize);

    if (fromX < 0 || fromX > m_GridWidth - 1 ||
        fromZ < 0 || fromZ > m_GridHeight - 1) {
        return false; // outside of map region !
    }

    if (toX < 0 || toX > m_GridWidth - 1 ||
        toZ < 0 || toZ > m_GridHeight - 1) {
        return false; // outside of map region !
    }

    GridNode* startNode = &m_AllNodes[(fromZ * m_GridWidth) + fromX];
    GridNode* endNode = &m_AllNodes[(toZ * m_GridWidth) + toX];

    std::vector<GridNode*> openList;
    std::vector<GridNode*> closedList;

    openList.emplace_back(startNode);

    startNode->F = 0.0f;
    startNode->G = 0.0f;
    startNode->Parent = nullptr;

    GridNode* currentBestNode = nullptr;

    while (!openList.empty()) {
        currentBestNode = RemoveBestNode(openList);

        if (currentBestNode == endNode) {// we ’ve found the path !
            GridNode * node = endNode;
            while (node != nullptr) {
                outPath.PushWaypoint(node->Position);
                node = node->Parent; // Build up the waypoints
            }
            return true;
        } else {
            for (int i = 0; i < 4; ++i) {
                GridNode * neighbour = currentBestNode->Connected[i];
                if (!neighbour) { // might not be connected ...
                    continue;
                }

                bool inClosed = NodeInList(neighbour, closedList);
                if (inClosed) {
                    continue; // already discarded this neighbour ...
                }

                float h = Heuristic(neighbour, endNode);
                float g = currentBestNode->G + currentBestNode->Costs[i];
                float f = h + g;

                bool inOpen = NodeInList(neighbour, openList);

                if (!inOpen) { // first time we ’ve seen this neighbour
                    openList.emplace_back(neighbour);
                }

                // might be a better route to this node !
                if (!inOpen || f < neighbour->F) {
                    neighbour->Parent = currentBestNode;
                    neighbour->F = f;
                    neighbour->G = g;
                }
            }
            closedList.emplace_back(currentBestNode);
        }
    }

    return false; //open list emptied out with no path!
}

void NavigationGrid::DebugDrawGrid() {
    //now to build the connectivity between the nodes
    for (int y = 0; y < m_GridHeight; ++y) {
        for (int x = 0; x < m_GridWidth; ++x) {
            GridNode& n = m_AllNodes[(m_GridWidth * y) + x];

            if (n.Type == 'x') {
                Debug::DrawCube(n.Position, 5.0f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
            } else {
                Debug::DrawCube(n.Position, 2.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
            }
        }
    }
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
    std::vector<GridNode*>::iterator it = std::find(list.begin(), list.end(), n);
    return it != list.end();
}

GridNode* NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
    std::vector < GridNode* >::iterator bestI = list.begin();

    GridNode * bestNode = *list.begin();

    for (auto i = list.begin(); i != list.end(); ++i) {
        if ((*i)->F < bestNode->F) {
            bestNode = (*i);
            bestI = i;
        }
    }
    list.erase(bestI);

    return bestNode;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
    return (hNode->Position - endNode->Position).Length();
}
