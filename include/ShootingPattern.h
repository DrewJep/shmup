#ifndef SHOOTING_PATTERN_H
#define SHOOTING_PATTERN_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Projectile;

// Abstract base for enemy shooting behavior
class ShootingPattern {
public:
    virtual ~ShootingPattern() = default;

    // Called each frame; implementations may push new projectiles into the list
    virtual void update(float deltaTime,
                        const sf::Vector2f& enemyPos,
                        const sf::Vector2f& playerPos,
                        std::vector<std::unique_ptr<Projectile>>& projectiles) = 0;
};

// Factory helpers (implemented in ShootingPattern.cpp)
std::unique_ptr<ShootingPattern> makeDirectAtPlayerPattern(float fireRate = 1.0f, float projSpeed = 220.0f, float activeRadius = 400.0f, bool always = false);
std::unique_ptr<ShootingPattern> makeRadialPattern(int count = 8, float interval = 2.0f, float projSpeed = 160.0f);

#endif // SHOOTING_PATTERN_H
