#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Actor.h"
#include "Path.h"

class Projectile;
class ShootingPattern;

// Base class for all enemies. Subclasses own visuals and per-type tick logic (animation, etc.).
class Enemy : public Actor {
public:
    virtual ~Enemy();

    void update(float deltaTime, int screenWidth, int screenHeight, const sf::Vector2f& playerPos,
                std::vector<std::unique_ptr<Projectile>>& projectiles);
    void draw(sf::RenderWindow& window);

    void setHealth(int h);
    void setPath(std::unique_ptr<Path> p);
    bool hasPath() const;
    void setShootingPattern(std::unique_ptr<ShootingPattern> p);

    sf::FloatRect getBounds() const override;

protected:
    Enemy(float x, float y, float speed);

    float speed;
    int maxHealth;

    float movementTimer;
    float directionChangeInterval;
    std::unique_ptr<Path> path;
    std::unique_ptr<ShootingPattern> shooter;

    std::unique_ptr<sf::Sprite> sprite;

    virtual void updateVisuals(float deltaTime) = 0;

    void updateMovement(float deltaTime, int screenWidth, int screenHeight);
    void syncSpritePosition();

private:
    void tickShooter(float deltaTime, const sf::Vector2f& playerPos,
                     std::vector<std::unique_ptr<Projectile>>& projectiles);
};

#endif
