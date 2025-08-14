#pragma once
#include "GameObject.h"

class Bullet : public GameObject {
public:
    Bullet(float startX, float startY, Direction dir, bool fromPlayer);
    void update(float deltaTime) override;
    bool isDestroyed() const override { return destroyed; }
    
    bool isFromPlayer() const { return fromPlayer; }
    int getDamage() const { return damage; }
    void destroy() { destroyed = true; }
    Direction getDirection() const { return direction; }

private:
    Direction direction;
    bool fromPlayer;
    float speed;
    int damage;
    bool destroyed = false;
};
