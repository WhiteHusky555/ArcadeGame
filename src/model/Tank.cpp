#include "Tank.h"

Tank::Tank(float startX, float startY, Direction startDir, bool isPlayer)
  : GameObject(startX, startY), direction(startDir), player(isPlayer),
    maxHealth(100), health(100), speed(player ? 4.0f : 3.0f), reloadTime(player ? 0.8f : 1.5f), timeSinceLastShot(reloadTime) // Увеличена скорость с 2.5f/1.8f до 4.0f/3.0f
    {} // Танк игрока немного быстрее и стреляет быстрее

void Tank::update(float deltaTime) {
  if (timeSinceLastShot < reloadTime) {
      timeSinceLastShot += deltaTime;
  }
}

bool Tank::canFire() const {
  return timeSinceLastShot >= reloadTime;
}

void Tank::fire() {
  // Фактическое создание пули обрабатывается GameModel.
  // Этот метод просто отмечает, что танк выстрелил и сбрасывает его таймер.
  if (canFire()) {
      timeSinceLastShot = 0.0f;
  }
}
