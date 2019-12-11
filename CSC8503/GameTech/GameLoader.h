#pragma once
#include "../../Common/Vector3.h"
#include "../CSC8503Common/NavigationGrid.h"

#include <vector>

#define PATHFINDING_SUFFIX "_path"
#define MAP_FILE_EXTENTION ".gmap"

enum class MapTile {
    Ground,
    Water
};

class GameState {
public:
    GameState() = delete;
    GameState(const char* filePath);
    ~GameState() = default;

    uint8_t* GetHeightMap() const { return m_HeightMap; }
    MapTile* GetMapTiles() const { return m_Map; }

    NCL::CSC8503::NavigationGrid* GetNavigationGrid() { return m_PathfindMap; }

    unsigned int GetNodeSize() {return m_NodeSize;}
    unsigned int GetMapWidth() {return m_MapWidth;}
    unsigned int GetMapHeight() {return m_MapHeight;}

    uint8_t HeightAt(float x, float y) const {
        unsigned int xIndex, yIndex;

        xIndex = static_cast<unsigned int>(x / m_MapWidth);
        yIndex = static_cast<unsigned int>(y / m_MapWidth);

        return m_HeightMap[(yIndex * m_MapWidth) + xIndex];
    }

private:
    unsigned int m_NodeSize;
    unsigned int m_MapWidth;
    unsigned int m_MapHeight;

    uint8_t* m_HeightMap;
    MapTile* m_Map;

    NCL::CSC8503::NavigationGrid* m_PathfindMap;

    void LoadMapFromFile(const char* fullPath);
};
