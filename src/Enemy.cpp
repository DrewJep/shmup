#include "Enemy.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "ShootingPattern.h"

Enemy::~Enemy() = default;

Enemy::Enemy(float x, float y, float speed)
    : Actor(x, y, 1),
      speed(speed),
      maxHealth(1),
      movementTimer(0.0f),
      directionChangeInterval(1.0f + (std::rand() % 200) / 100.0f) {
    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
}

void Enemy::setHealth(int h) {
    health = h;
    maxHealth = std::max(1, h);
}

void Enemy::syncSpritePosition() {
    if (sprite) {
        sprite->setPosition(position);
    }
}

void Enemy::updateMovement(float deltaTime, int screenWidth, int screenHeight) {
    if (path) {
        path->update(deltaTime);
        position = path->getPosition();
        return;
    }

    movementTimer += deltaTime;

    if (movementTimer >= directionChangeInterval) {
        movementTimer = 0.0f;

        float centerX = screenWidth * 0.7f;
        float centerY = screenHeight / 2.0f;

        float targetAngle = std::atan2(centerY - position.y, centerX - position.x);
        float variation = (std::rand() % 90 - 45) * 3.14159f / 180.0f;
        targetAngle += variation;

        velocity.x = std::cos(targetAngle) * speed;
        velocity.y = std::sin(targetAngle) * speed;
    }
}

void Enemy::tickShooter(float deltaTime, const sf::Vector2f& playerPos,
                        std::vector<std::unique_ptr<Projectile>>& projectiles) {
    if (shooter) {
        shooter->update(deltaTime, position, playerPos, projectiles);
    }
}

void Enemy::update(float deltaTime, int screenWidth, int screenHeight, const sf::Vector2f& playerPos,
                     std::vector<std::unique_ptr<Projectile>>& projectiles) {
    bool followingPath = (path != nullptr);
    updateMovement(deltaTime, screenWidth, screenHeight);

    if (!followingPath) {
        position += velocity * deltaTime;
    }

    syncSpritePosition();
    updateVisuals(deltaTime);
    tickShooter(deltaTime, playerPos, projectiles);
}

void Enemy::setPath(std::unique_ptr<Path> p) {
    path = std::move(p);
    if (path) {
        path->setStart(position);
        velocity = sf::Vector2f(0.f, 0.f);
    }
}

void Enemy::setShootingPattern(std::unique_ptr<ShootingPattern> p) {
    shooter = std::move(p);
}

bool Enemy::hasPath() const {
    return path != nullptr && !path->isFinished();
}

void Enemy::draw(sf::RenderWindow& window) {
    if (sprite) window.draw(*sprite);
}

sf::FloatRect Enemy::getBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    return sf::FloatRect({position.x, position.y}, {0.f, 0.f});
}
