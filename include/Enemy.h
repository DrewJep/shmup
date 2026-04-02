#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Actor.h"
#include "Path.h"

// Forward declarations
class Projectile;
class ShootingPattern;

class Enemy : public Actor {
public:
    enum class Type { UFO, Tank };

    // type controls which sprite/behavior to use (UFO is default)
    Enemy(float x, float y, float speed = 100.0f, Type type = Type::UFO);

    // Now accepts player position and projectiles list so enemies can spawn bullets
    void update(float deltaTime, int screenWidth, int screenHeight, const sf::Vector2f& playerPos,
                std::vector<std::unique_ptr<Projectile>>& projectiles);
    void draw(sf::RenderWindow& window);

    void setHealth(int h);
    // Path movement
    void setPath(std::unique_ptr<Path> p);
    bool hasPath() const;
    // Shooting pattern
    void setShootingPattern(std::unique_ptr<ShootingPattern> p);

    sf::FloatRect getBounds() const override;

private:
    float speed;
    int maxHealth;

    // Animation (textures owned by TextureCache)
    sf::Texture* sheetTexture = nullptr; // UFO sheet or single-frame tank
    static const int FRAME_COLS = 2;
    static const int FRAME_ROWS = 3;
    static const int TOTAL_FRAMES = FRAME_COLS * FRAME_ROWS;

    std::unique_ptr<sf::Sprite> sprite;
    int currentFrame;
    float animationTimer;
    float frameDuration;
    Type enemyType;

    // Movement pattern
    float movementTimer;
    float directionChangeInterval;
    std::unique_ptr<Path> path;
    std::unique_ptr<ShootingPattern> shooter;

    // Internal helpers
    void updateAnimation(float deltaTime);
    void updateSpriteRect();
    void updateMovement(float deltaTime, int screenWidth, int screenHeight);
};

#endif // ENEMY_H
