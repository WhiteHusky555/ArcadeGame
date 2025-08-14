#pragma once
#include <vector>
#include <string>
#include "TileType.h"

class GameMap {
public:
    bool loadFromFile(const std::string& filename);
    TileType getTile(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    void setTile(int x, int y, TileType tile);
    void resetToInitialState();

    std::vector<std::pair<int, int>> enemyStarts;
    std::pair<int, int> playerStart;

private:
    std::vector<std::vector<TileType>> grid;
    std::vector<std::vector<TileType>> originalGrid;
};
