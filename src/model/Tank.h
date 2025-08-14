#pragma once
#include "../model/GameObject.h"
#include "../common/Direction.h"

class Tank : public GameObject {
public:
  Tank(float startX, float startY, Direction startDir, bool isPlayer);
  void update(float deltaTime) override;
  bool isDestroyed() const override { return health <= 0; }
  
  void fire(); // Call this to attempt firing, resets reload timer
  bool canFire() const; // Check if tank is ready to fire
  void takeDamage(int damage) { 
      health -= damage; 
      if (health < 0) health = 0;
  }
  
  Direction getDirection() const { return direction; }
  void setDirection(Direction newDir) { direction = newDir; }
  bool isPlayer() const { return player; }
  int getHealth() const { return health; }
  int getMaxHealth() const { return maxHealth; }
  float getSpeed() const { return speed; }

private:
  Direction direction;
  bool player;
  int health;
  int maxHealth;
  float speed;
  float reloadTime; // Time in seconds between shots
  float timeSinceLastShot; // Time elapsed since last shot
};
