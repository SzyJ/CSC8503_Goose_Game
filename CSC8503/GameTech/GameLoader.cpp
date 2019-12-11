#include "GameLoader.h"
#include "../../Common/Assets.h"

#include <fstream>

GameState::GameState(const char* filePath) {
    std::string mapFilepath = NCL::Assets::DATADIR + filePath + MAP_FILE_EXTENTION;
    std::string pathfindFilepath = NCL::Assets::DATADIR + filePath + PATHFINDING_SUFFIX + MAP_FILE_EXTENTION;

    m_PathfindMap = new NCL::CSC8503::NavigationGrid(pathfindFilepath);

    LoadMapFromFile(mapFilepath.c_str());
}

void GameState::LoadMapFromFile(const char* fullPath) {
    std::ifstream infile(fullPath);
    
    infile >> m_NodeSize;
    infile >> m_MapWidth;
    infile >> m_MapHeight;

    m_Map = new MapTile[m_MapWidth * m_MapHeight];
    m_HeightMap = new uint8_t[m_MapWidth * m_MapHeight];

    for (int y = 0; y < m_MapHeight; ++y) {
        std::string mapRow;
        infile >> mapRow;

        for (int x = 0; x < m_MapWidth; ++x) {
            char thisTile = mapRow[x];
            unsigned int index = (y * m_MapWidth) + x;

            if (isdigit(thisTile)) {
                m_HeightMap[index] = static_cast<uint8_t>(thisTile) - 48;
                m_Map[index] = MapTile::Ground;
                continue;
            }

            if (thisTile == 'w') {
                m_HeightMap[index] = 0;
                m_Map[index] = MapTile::Water;
            }
        }
    }
}
