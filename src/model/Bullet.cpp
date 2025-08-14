#include "Bullet.h"

Bullet::Bullet(float startX, float startY, Direction dir, bool fromPlayer)
    : GameObject(startX, startY), direction(dir), fromPlayer(fromPlayer),
      speed(8.0f), damage(25), destroyed(false) {}  // Увеличена скорость с 5.0f до 8.0f

void Bullet::update(float deltaTime) {
    switch(direction) {
        case Direction::UP:    y -= speed; break;
        case Direction::DOWN:  y += speed; break;
        case Direction::LEFT:  x -= speed; break;
        case Direction::RIGHT: x += speed; break;
    }
}
