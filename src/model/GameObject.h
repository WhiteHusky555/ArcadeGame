#pragma once
#include "../common/Direction.h"
#include <memory>

class GameObject {
public:
    GameObject(float x, float y);
    virtual ~GameObject() = default;
    
    virtual void update(float deltaTime) = 0;
    virtual bool isDestroyed() const = 0;
    
    float getX() const { return x; }
    float getY() const { return y; }
    void setPosition(float newX, float newY) { x = newX; y = newY; }

protected:
    float x, y;
};
