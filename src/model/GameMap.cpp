#include "GameMap.h"
#include <fstream>
#include <stdexcept> // Для std::runtime_error

bool GameMap::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    enemyStarts.clear();
    playerStart = {-1, -1}; // Инициализируем в недопустимое состояние

    grid.clear();
    std::string line;
    int firstLineWidth = -1;
    int currentLineNumber = 0;

    while (std::getline(file, line)) {
        if (line.empty() && file.eof()) { // Разрешаем одну завершающую пустую строку, если это самый конец
            break;
        }
        if (currentLineNumber == 0) {
            firstLineWidth = static_cast<int>(line.length());
            if (firstLineWidth == 0) {
                file.close();
                return false;
            }
        } else {
            if (static_cast<int>(line.length()) != firstLineWidth) {
                file.close();
                return false;
            }
        }

        std::vector<TileType> row;
        for (int col = 0; col < static_cast<int>(line.length()); ++col) {
            char c = line[col];
            if (c == '#') {
                row.push_back(TileType::Wall);
            } else if (c == 'P') {
                if (playerStart.first == -1) { // Устанавливаем только для первого найденного 'P'
                    playerStart = {col, currentLineNumber};
                }
                row.push_back(TileType::Empty);
            } else if (c == 'E') {
                enemyStarts.push_back({col, currentLineNumber});
                row.push_back(TileType::Empty);
            } else {
                row.push_back(TileType::Empty);
            }
        }
        grid.push_back(row);
        currentLineNumber++;
    }
    file.close();

    if (grid.empty()) {
        return false;
    }

    // Проверяем стартовую позицию игрока
    if (playerStart.first < 0 || playerStart.first >= getWidth() || 
        playerStart.second < 0 || playerStart.second >= getHeight()) {
        return false;
    }

    // Проверяем стартовые позиции врагов
    for (const auto& pos : enemyStarts) {
        if (pos.second >= static_cast<int>(grid.size()) || pos.first >= static_cast<int>(grid[0].size())) {
            return false; // Или удалить эту конкретную стартовую позицию врага и продолжить
        }
    }

    originalGrid = grid;
    return true;
}

TileType GameMap::getTile(int x, int y) const {
    if (y >= 0 && y < static_cast<int>(grid.size()) && 
        x >= 0 && x < static_cast<int>(grid[y].size())) {
        return grid[y][x];
    }
    // Не должно происходить, если координаты проверены, но как запасной вариант:
    return TileType::Wall; // Считаем за пределами как стену для безопасности
}

int GameMap::getWidth() const {
    return grid.empty() ? 0 : static_cast<int>(grid[0].size());
}

int GameMap::getHeight() const {
    return static_cast<int>(grid.size());
}

void GameMap::setTile(int x, int y, TileType tile) {
    if (y >= 0 && y < static_cast<int>(grid.size()) && 
        x >= 0 && x < static_cast<int>(grid[y].size())) {
        grid[y][x] = tile;
    }
}

void GameMap::resetToInitialState() {
    grid = originalGrid;
}
