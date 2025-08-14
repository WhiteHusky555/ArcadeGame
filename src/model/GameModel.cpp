#include "GameModel.h"
#include "../model/Bullet.h"
#include "../model/Tank.h"
#include <algorithm> // Для std::shuffle, std::remove_if
#include <cmath>
#include <cstdlib> // Для rand, srand
#include <ctime>   // Для time
#include <random>  // Для std::mt19937, std::shuffle

GameModel::GameModel() : playerTank(nullptr) {
    lastUpdateTime = std::chrono::steady_clock::now();
    // Инициализируем генератор случайных чисел один раз
    srand(static_cast<unsigned int>(time(nullptr)));
}

bool GameModel::init(const std::string& mapFile) {
    if (!gameMap.loadFromFile(mapFile)) {
        return false;
    }
    reset();
    return true;
}

void GameModel::reset() {
    gameObjects.clear();
    playerTank = nullptr; // Явно обнуляем перед переназначением

    // Проверяем координаты стартовой позиции игрока относительно текущих размеров карты
    if (gameMap.playerStart.first < 0 || gameMap.playerStart.first >= gameMap.getWidth() ||
        gameMap.playerStart.second < 0 || gameMap.playerStart.second >= gameMap.getHeight()) {
        // Это состояние должно в идеале предотвратить продолжение игры
        // Пока что продолжаем, но это указывает на критическую проблему, если происходит
        return;
    }

    float playerPixelX = gameMap.playerStart.first * TILE_SIZE;
    float playerPixelY = gameMap.playerStart.second * TILE_SIZE;

    // Дополнительная проверка
    if (playerPixelX < 0 || playerPixelY < 0 ||
        playerPixelX + TANK_SIZE > gameMap.getWidth() * TILE_SIZE ||
        playerPixelY + TANK_SIZE > gameMap.getHeight() * TILE_SIZE) {
        return;
    }

    auto player = std::make_unique<Tank>(
        playerPixelX,
        playerPixelY,
        Direction::UP,
        true
    );
    playerTank = player.get(); // Получаем указатель на танк игрока
    gameObjects.push_back(std::move(player));

    // Создаем вражеские танки
    for (const auto& pos : gameMap.enemyStarts) {
        // Проверяем координаты стартовых позиций врагов
        if (pos.first < 0 || pos.first >= gameMap.getWidth() ||
            pos.second < 0 || pos.second >= gameMap.getHeight()) {
            continue;
        }
        float enemyPixelX = pos.first * TILE_SIZE + (TILE_SIZE - TANK_SIZE) / 2.0f;
        float enemyPixelY = pos.second * TILE_SIZE + (TILE_SIZE - TANK_SIZE) / 2.0f;

        gameObjects.push_back(std::make_unique<Tank>(
            enemyPixelX,
            enemyPixelY,
            static_cast<Direction>(rand() % 4), // Случайное начальное направление
            false
        ));
    }

    state = GameState::PLAYING; // Явно устанавливаем состояние PLAYING при сбросе
    score = 0;
    gameTime = 0;
    gameMap.resetToInitialState(); // Сбрасываем тайлы карты, если они могут быть изменены
    lastUpdateTime = std::chrono::steady_clock::now(); // Сбрасываем время для расчета deltaTime
}

bool GameModel::checkWallCollision(float x, float y, float width, float height) const {
    const float mapWidth = gameMap.getWidth() * TILE_SIZE;
    const float mapHeight = gameMap.getHeight() * TILE_SIZE;

    // Проверяем границы карты
    if (x < 0 || y < 0 || (x + width) > mapWidth || (y + height) > mapHeight) {
        return true;
    }

    // Вычисляем диапазон покрываемых тайлов с безопасным эпсилоном
    const int startTileX = static_cast<int>(x / TILE_SIZE);
    const int startTileY = static_cast<int>(y / TILE_SIZE);
    const int endTileX = static_cast<int>((x + width - 0.001f) / TILE_SIZE);
    const int endTileY = static_cast<int>((y + height - 0.001f) / TILE_SIZE);

    // Проверяем все тайлы в вычисленном диапазоне
    for (int ty = startTileY; ty <= endTileY; ++ty) {
        for (int tx = startTileX; tx <= endTileX; ++tx) {
            // Защита от потенциальных ошибок округления
            if (tx < 0 || tx >= gameMap.getWidth() || 
                ty < 0 || ty >= gameMap.getHeight()) {
                // Этот случай в идеале должен быть перехвачен начальной проверкой границ,
                // но хорош для надежности, если объект частично внутри/снаружи
                continue; 
            }
            
            if (gameMap.getTile(tx, ty) == TileType::Wall) {
                return true;
            }
        }
    }
    
    return false; // Нет коллизий после проверки всех тайлов
}

void GameModel::update() {
    if (state != GameState::PLAYING) {
        // Если игра окончена и playerTank существует, убеждаемся, что он помечен как уничтоженный
        // Это может быть избыточно, если isDestroyed() точен, но как защита:
        if (state == GameState::GAME_OVER && playerTank && !playerTank->isDestroyed()) {
            // Этот случай в идеале не должен происходить, если playerTank->isDestroyed() корректно запускает GAME_OVER
        }
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime);
    float deltaTime = elapsed.count() / 1000.0f;
    lastUpdateTime = now;
    gameTime += deltaTime;
  
    if (deltaTime > 0.0001f) { // Избегаем деления на ноль или крайне малого deltaTime
        fps = 1.0f / deltaTime;
    } else {
        fps = 0; // Или какое-то высокое значение, но 0 указывает на проблему или фактически приостановленное состояние
    }

    // Обновляем все игровые объекты
    for (auto& obj : gameObjects) {
        obj->update(deltaTime);
    }

    updateEnemies(deltaTime); // Логика ИИ для врагов
    processCollisions();    // Обрабатываем взаимодействия и урон

    // Проверяем смерть игрока *перед* удалением объектов, чтобы playerTank был еще действителен
    // и его состояние можно было проверить через isDestroyed()
    if (playerTank && playerTank->isDestroyed()) {
        state = GameState::GAME_OVER;
        // Не возвращаемся здесь, позволяем удалить объект танка игрока ниже, если он в gameObjects
    }

    // Удаляем уничтоженные объекты
    // Будьте осторожны, если playerTank указывает на объект, который собирается быть удален
    gameObjects.erase(
        std::remove_if(gameObjects.begin(), gameObjects.end(),
            [&](const std::unique_ptr<GameObject>& obj) { 
                bool destroyed = obj->isDestroyed();
                // Если объект playerTank уничтожен и удален, указатель playerTank будет висячим
                // GameState::GAME_OVER должно предотвратить дальнейшие действия, зависящие от состояния объекта playerTank
                // Доступ к playerTank->getHealth() через getPlayerHealth() сделан безопасным проверкой состояния игры
                return destroyed; 
            }),
        gameObjects.end()
    );

    // После удаления, если playerTank был уничтожен, его объект больше не в gameObjects
    // Указатель playerTank будет висячим
    // GameState::GAME_OVER должно предотвратить дальнейшие действия, зависящие от playerTank
}

void GameModel::playerMove(Direction dir) {
    if (playerTank && !playerTank->isDestroyed() && state == GameState::PLAYING) {
        float currentX = playerTank->getX();
        float currentY = playerTank->getY();
        float speed = playerTank->getSpeed();

        float potentialX = currentX;
        float potentialY = currentY;

        playerTank->setDirection(dir); // Устанавливаем направление независимо от движения

        switch (dir) {
            case Direction::UP:    potentialY -= speed; break;
            case Direction::DOWN:  potentialY += speed; break;
            case Direction::LEFT:  potentialX -= speed; break;
            case Direction::RIGHT: potentialX += speed; break;
        }

        if (!checkWallCollision(potentialX, potentialY, TANK_SIZE, TANK_SIZE)) {
            playerTank->setPosition(potentialX, potentialY);
        }
    }
}

void GameModel::playerFire() {
    if (playerTank && playerTank->canFire() && state == GameState::PLAYING) { // Проверяем, может ли танк стрелять
        playerTank->fire(); // Это сбросит внутренний таймер стрельбы танка
    
        float bulletX = playerTank->getX() + TANK_SIZE / 2.0f; // Начинаем из центра танка
        float bulletY = playerTank->getY() + TANK_SIZE / 2.0f;
        Direction dir = playerTank->getDirection();
    
        // Корректируем начальную позицию пули, чтобы она была на краю танка, перед башней
        float offset = TANK_SIZE / 2.0f + 1.0f; // Небольшое смещение, чтобы очистить корпус танка
        switch (dir) {
            case Direction::UP:    bulletY -= offset; break;
            case Direction::DOWN:  bulletY += offset; break;
            case Direction::LEFT:  bulletX -= offset; break;
            case Direction::RIGHT: bulletX += offset; break;
        }
    
        addBullet(std::make_unique<Bullet>(bulletX, bulletY, dir, true));
    }
}

void GameModel::addBullet(std::unique_ptr<Bullet> bullet) {
    gameObjects.push_back(std::move(bullet));
}

bool GameModel::isCellFree(float x, float y) const {
    // Эта функция проверяет одну точку. Может потребоваться обновление или удаление
    // если checkWallCollision используется везде для проверки областей
    int tileX = static_cast<int>(x / TILE_SIZE);
    int tileY = static_cast<int>(y / TILE_SIZE);
    if (tileX < 0 || tileX >= gameMap.getWidth() || tileY < 0 || tileY >= gameMap.getHeight()) {
        return false; // За пределами карты не свободно
    }
    return gameMap.getTile(tileX, tileY) != TileType::Wall;
}

int GameModel::getPlayerHealth() const {
    if (state == GameState::GAME_OVER) return 0; // Если игра окончена, здоровье игрока фактически 0
    return playerTank ? playerTank->getHealth() : 0;
}

bool GameModel::isPlayerDead() const {
    // Этот метод в основном для GameWindow для проверки
    // Внутренняя логика GameModel должна полагаться на playerTank->isDestroyed() и затем устанавливать GameState
    if (state == GameState::GAME_OVER) return true; // Если игра окончена, игрок считается мертвым
    if (!playerTank) return true; // Не должно происходить, если игра была в процессе и состояние не GAME_OVER
    return playerTank->isDestroyed();
}

void GameModel::updateEnemies(float deltaTime) {
    for (auto& obj : gameObjects) {
        Tank* tank = dynamic_cast<Tank*>(obj.get());
        if (tank && !tank->isPlayer() && !tank->isDestroyed() && state == GameState::PLAYING) {
            // Движение ИИ
            if (rand() % 150 < 5) { // Корректируем частоту принятия решений о движении
                Direction moveDir = static_cast<Direction>(rand() % 4);
            
                float currentX = tank->getX();
                float currentY = tank->getY();
                float speed = tank->getSpeed();

                float potentialX = currentX;
                float potentialY = currentY;
            
                tank->setDirection(moveDir);

                // Пытаемся двигаться на долю размера танка для дискретного шага
                constexpr float testMoveAmount = TANK_SIZE / 4.0f; 

                switch (moveDir) {
                    case Direction::UP:    potentialY = currentY - testMoveAmount; break;
                    case Direction::DOWN:  potentialY = currentY + testMoveAmount; break;
                    case Direction::LEFT:  potentialX = currentX - testMoveAmount; break;
                    case Direction::RIGHT: potentialX = currentX + testMoveAmount; break;
                }
            
                if (!checkWallCollision(potentialX, potentialY, TANK_SIZE, TANK_SIZE)) {
                    // Проверяем коллизию с другими танками перед движением
                    bool collisionWithOtherTank = false;
                    for (const auto& otherObj : gameObjects) {
                        if (otherObj.get() == tank) continue; // Пропускаем себя
                        const Tank* otherTank = dynamic_cast<const Tank*>(otherObj.get());
                        if (otherTank && !otherTank->isDestroyed()) {
                            float otherLeft = otherTank->getX();
                            float otherRight = otherTank->getX() + TANK_SIZE;
                            float otherTop = otherTank->getY();
                            float otherBottom = otherTank->getY() + TANK_SIZE;

                            if (potentialX + TANK_SIZE > otherLeft && potentialX < otherRight &&
                                potentialY + TANK_SIZE > otherTop && potentialY < otherBottom) {
                                collisionWithOtherTank = true;
                                break;
                            }
                        }
                    }
                    if (!collisionWithOtherTank) {
                       tank->setPosition(potentialX, potentialY);
                    }
                }
            }
        
            // Стрельба ИИ
            if (rand() % 100 < 2 && tank->canFire()) { // Проверяем, может ли танк стрелять
                tank->fire(); // Сбрасываем таймер стрельбы танка
            
                float bulletX = tank->getX() + TANK_SIZE / 2.0f;
                float bulletY = tank->getY() + TANK_SIZE / 2.0f;
                Direction dir = tank->getDirection();
            
                float offset = TANK_SIZE / 2.0f + 1.0f;
                switch (dir) {
                    case Direction::UP:    bulletY -= offset; break;
                    case Direction::DOWN:  bulletY += offset; break;
                    case Direction::LEFT:  bulletX -= offset; break;
                    case Direction::RIGHT: bulletX += offset; break;
                }
                addBullet(std::make_unique<Bullet>(bulletX, bulletY, dir, false));
            }
        }
    }
}

void GameModel::processCollisions() {
    // Коллизии пуль (итерируем осторожно, так как пули могут быть уничтожены)
    for (auto it_bullet = gameObjects.begin(); it_bullet != gameObjects.end(); ) {
        Bullet* bullet = dynamic_cast<Bullet*>(it_bullet->get());
        if (!bullet || bullet->isDestroyed()) {
            ++it_bullet;
            continue;
        }
    
        // Коллизия пули со стеной
        int tileX = static_cast<int>(std::floor(bullet->getX() / TILE_SIZE));
        int tileY = static_cast<int>(std::floor(bullet->getY() / TILE_SIZE));

        if (tileX < 0 || tileX >= gameMap.getWidth() || tileY < 0 || tileY >= gameMap.getHeight() ||
            gameMap.getTile(tileX, tileY) == TileType::Wall) {
            bullet->destroy();
            ++it_bullet; // Переходим к следующему объекту после обработки текущей пули
            continue;
        }
    
        // Коллизия пули с танком
        bool bullet_hit_tank = false;
        for (auto it_tank = gameObjects.begin(); it_tank != gameObjects.end(); ++it_tank) {
            // Не перепроверяем саму пулю, если она в том же цикле (не проблема здесь из-за dynamic_cast)
            if (it_tank->get() == bullet) continue;

            Tank* tank = dynamic_cast<Tank*>(it_tank->get());
            if (!tank || tank->isDestroyed()) continue;
        
            // Предотвращаем дружественный огонь или самоповреждение пулями
            if (bullet->isFromPlayer() == tank->isPlayer()) continue;
        
            // Проверка коллизии AABB для пули и танка
            float bulletLeft = bullet->getX() - 3; // Радиус пули равен 3
            float bulletRight = bullet->getX() + 3;
            float bulletTop = bullet->getY() - 3;
            float bulletBottom = bullet->getY() + 3;

            float tankLeft = tank->getX();
            float tankRight = tank->getX() + TANK_SIZE;
            float tankTop = tank->getY();
            float tankBottom = tank->getY() + TANK_SIZE;

            if (bulletRight > tankLeft && bulletLeft < tankRight &&
                bulletBottom > tankTop && bulletTop < tankBottom) {
                tank->takeDamage(bullet->getDamage());
                bullet->destroy();
                bullet_hit_tank = true; // Помечаем пулю для уничтожения
                          
                if (bullet->isFromPlayer() && tank->isDestroyed()) {
                    score += 100;
                    spawnNewEnemyRandomly(); // Создаем нового врага
                }
                // Если вражеская пуля убила игрока, playerTank->isDestroyed() будет true
                // GameModel::update() установит GameState::GAME_OVER
                break; // Пуля попадает в один танк и уничтожается
            }
        }
        if (bullet_hit_tank) {
            // Пуля уже помечена как уничтоженная, будет очищена основным циклом erase
        }
        ++it_bullet; // Переходим к следующему объекту
    } // Конец цикла коллизий пуль
    
    // Коллизии танк-танк (простое расталкивание) - выполняется после коллизий пуль
    for (size_t i = 0; i < gameObjects.size(); ++i) {
        Tank* tank1 = dynamic_cast<Tank*>(gameObjects[i].get());
        if (!tank1 || tank1->isDestroyed()) continue;
          
        for (size_t j = i + 1; j < gameObjects.size(); ++j) {
            Tank* tank2 = dynamic_cast<Tank*>(gameObjects[j].get());
            if (!tank2 || tank2->isDestroyed()) continue;

            float dx = (tank1->getX() + TANK_SIZE/2.0f) - (tank2->getX() + TANK_SIZE/2.0f); // От центра к центру
            float dy = (tank1->getY() + TANK_SIZE/2.0f) - (tank2->getY() + TANK_SIZE/2.0f);
            float distance = std::sqrt(dx*dx + dy*dy);
            float min_dist = TANK_SIZE; // Минимальное расстояние до того, как они считаются перекрывающимися

            if (distance < min_dist && distance > 0.001f) { // Если перекрываются и не идеально совпадают
                float overlap = TANK_SIZE - distance;
                float pushX = (dx / distance) * overlap / 2.0f; // Толкаем на половину перекрытия
                float pushY = (dy / distance) * overlap / 2.0f;
                
                // Предварительные новые позиции
                float tank1NewX = tank1->getX() + pushX;
                float tank1NewY = tank1->getY() + pushY;
                float tank2NewX = tank2->getX() - pushX;
                float tank2NewY = tank2->getY() - pushY;
                
                // Проверяем коллизии перед применением толчка, чтобы предотвратить толкание в стены
                // Это упрощенная модель
                if (!checkWallCollision(tank1NewX, tank1NewY, TANK_SIZE, TANK_SIZE)) {
                    tank1->setPosition(tank1NewX, tank1NewY);
                } else if (!checkWallCollision(tank2->getX(), tank2->getY(), TANK_SIZE, TANK_SIZE)) { 
                    // Если tank1 не может двигаться, пытаемся двигать только tank2 от исходной позиции tank1
                    float tank2NewX_alt = tank2->getX() - 2*pushX; // Толкаем tank2 на полное перекрытие
                    float tank2NewY_alt = tank2->getY() - 2*pushY;
                     if (!checkWallCollision(tank2NewX_alt, tank2NewY_alt, TANK_SIZE, TANK_SIZE)) {
                         tank2->setPosition(tank2NewX_alt, tank2NewY_alt);
                     }
                }

                if (!checkWallCollision(tank2NewX, tank2NewY, TANK_SIZE, TANK_SIZE)) {
                    tank2->setPosition(tank2NewX, tank2NewY);
                } else if (!checkWallCollision(tank1->getX(), tank1->getY(), TANK_SIZE, TANK_SIZE)) {
                    float tank1NewX_alt = tank1->getX() + 2*pushX;
                    float tank1NewY_alt = tank1->getY() + 2*pushY;
                    if (!checkWallCollision(tank1NewX_alt, tank1NewY_alt, TANK_SIZE, TANK_SIZE)) {
                        tank1->setPosition(tank1NewX_alt, tank1NewY_alt);
                    }
                }
            } else if (distance < 0.001f) { // Идеально совпадают, толкаем по оси x как запасной вариант
                  float tank1NewX_pc = tank1->getX() + TANK_SIZE / 4.0f; // Толкаем на небольшое количество
                  float tank2NewX_pc = tank2->getX() - TANK_SIZE / 4.0f;
                  if (!checkWallCollision(tank1NewX_pc, tank1->getY(), TANK_SIZE, TANK_SIZE)) {
                      tank1->setPosition(tank1NewX_pc, tank1->getY());
                  }
                  if (!checkWallCollision(tank2NewX_pc, tank2->getY(), TANK_SIZE, TANK_SIZE)) {
                      tank2->setPosition(tank2NewX_pc, tank2->getY());
                  }
            }
        }
    }
} // Конец цикла коллизий танк-танк

bool GameModel::findEmptySpawnLocation(float& outX, float& outY) {
    std::vector<std::pair<int, int>> possibleSpawns = gameMap.enemyStarts;
    if (possibleSpawns.empty()) {
        // Запасной вариант: если нет предопределенных стартов врагов, мы не можем создать используя этот метод
        // Более сложный метод мог бы найти любой пустой тайл на карте
        return false;
    }

    // Используем лучший генератор случайных чисел для перемешивания
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(possibleSpawns.begin(), possibleSpawns.end(), g);

    for (const auto& tilePos : possibleSpawns) {
        if (tilePos.first < 0 || tilePos.first >= gameMap.getWidth() ||
            tilePos.second < 0 || tilePos.second >= gameMap.getHeight()) {
            continue;
        }
        float potentialX = tilePos.first * TILE_SIZE + (TILE_SIZE - TANK_SIZE) / 2.0f;
        float potentialY = tilePos.second * TILE_SIZE + (TILE_SIZE - TANK_SIZE) / 2.0f;

        // Проверка 1: Является ли сам тайл стеной? (Не должно быть для enemyStarts, но хорошая проверка)
        if (gameMap.getTile(tilePos.first, tilePos.second) == TileType::Wall) {
            continue;
        }

        // Проверка 2: Занято ли местоположение (выровненное по тайлу) каким-либо существующим танком?
        bool occupied = false;
        for (const auto& obj : gameObjects) {
            const Tank* tank = dynamic_cast<const Tank*>(obj.get());
            if (tank && !tank->isDestroyed()) {
                // Проверка AABB на перекрытие (предполагая TANK_SIZE для коллизионного бокса)
                float otherTankLeft = tank->getX();
                float otherTankRight = tank->getX() + TANK_SIZE;
                float otherTankTop = tank->getY();
                float otherTankBottom = tank->getY() + TANK_SIZE;

                // Определяем ограничивающий бокс для потенциального места создания
                float spawnLeft = potentialX;
                float spawnRight = potentialX + TANK_SIZE;
                float spawnTop = potentialY;
                float spawnBottom = potentialY + TANK_SIZE;

                // Проверяем на перекрытие
                if (spawnRight > otherTankLeft && spawnLeft < otherTankRight &&
                    spawnBottom > otherTankTop && spawnTop < otherTankBottom) {
                    occupied = true;
                    break;
                }
            }
        }

        if (!occupied) {
            outX = potentialX;
            outY = potentialY;
            return true; // Нашли пустое место
        }
    }
    return false; // Не найдено подходящее пустое место для создания из enemyStarts
}

void GameModel::spawnNewEnemyRandomly() {
    float spawnX, spawnY;
    if (findEmptySpawnLocation(spawnX, spawnY)) {
        Direction randomDir = static_cast<Direction>(rand() % 4);
        gameObjects.push_back(std::make_unique<Tank>(spawnX, spawnY, randomDir, false));
    }
}
