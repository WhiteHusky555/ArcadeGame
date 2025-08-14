#pragma once
#include "GameMap.h"
#include "../common/Direction.h"
#include "GameObject.h"
#include "Tank.h"
#include "Bullet.h"
#include <vector>
#include <memory>
#include <chrono>
#include <cmath> 

enum class GameState {
PLAYING,
PAUSED,
GAME_OVER,
MENU 
};

class GameModel {
public:
static constexpr float TILE_SIZE = 40.0f;  // Увеличено с 20.0f до 40.0f
static constexpr float TANK_SIZE = 36.0f;  // Увеличено с 20.0f до 36.0f

GameModel();
bool init(const std::string& mapFile);
void update();
void reset();

GameState getState() const { return state; }
void setState(GameState newState) { state = newState; }

int getScore() const { return score; }
float getFPS() const { return fps; }
int getPlayerHealth() const;

const GameMap& getMap() const { return gameMap; }
const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return gameObjects; }

void playerMove(Direction dir);
void playerFire();
void addBullet(std::unique_ptr<Bullet> bullet);
bool isCellFree(float x, float y) const; // This might be superseded by checkWallCollision or need review

int getPlayerScore() const { return score; } 
bool isPlayerDead() const; 

private:
void processCollisions();

void updateEnemies(float deltaTime);
bool checkWallCollision(float x, float y, float width, float height) const;
void spawnNewEnemyRandomly(); // New function
bool findEmptySpawnLocation(float& outX, float& outY); // Helper for spawning

GameMap gameMap;
std::vector<std::unique_ptr<GameObject>> gameObjects;
Tank* playerTank;
GameState state = GameState::PLAYING; // Default to PLAYING, actual initial state set by controller
int score = 0;
float fps = 0;
float gameTime = 0;
std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
};
