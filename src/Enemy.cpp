#include "Enemy.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "ShootingPattern.h"
#include "TextureCache.h"

Enemy::Enemy(float x, float y, float speed, Type type)
    : Actor(x, y, 1),
      speed(speed),
      maxHealth(1),
      movementTimer(0.0f),
      directionChangeInterval(1.0f + (std::rand() % 200) / 100.0f),
      currentFrame(0),
      animationTimer(0.0f),
      frameDuration(0.08f),
      sprite(nullptr),
      enemyType(type) {
    auto& cache = TextureCache::instance();

    if (enemyType == Type::Tank) {
        sheetTexture = cache.load("assets/characters/tank.png");
        if (sheetTexture) {
            sprite = std::make_unique<sf::Sprite>(*sheetTexture);
            sprite->setOrigin(
                sf::Vector2f(sheetTexture->getSize().x / 2.f, sheetTexture->getSize().y / 2.f));
            sprite->setPosition(position);
        } else {
            sheetTexture = cache.load("assets/characters/ufo.png");
            if (sheetTexture) {
                sprite = std::make_unique<sf::Sprite>(*sheetTexture);
                updateSpriteRect();
                sf::Vector2u texSize = sheetTexture->getSize();
                int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
                int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;
                sprite->setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));
                sprite->setPosition(position);
            }
        }
    } else {
        sheetTexture = cache.load("assets/characters/ufo.png");
        if (sheetTexture) {
            sprite = std::make_unique<sf::Sprite>(*sheetTexture);
            updateSpriteRect();
            sf::Vector2u texSize = sheetTexture->getSize();
            int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
            int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;
            sprite->setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));
            sprite->setPosition(position);
        }
    }

    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
}

void Enemy::setHealth(int h) {
    health = h;
    maxHealth = std::max(1, h);
}

void Enemy::updateSpriteRect() {
    if (!sheetTexture || !sprite) return;

    sf::Vector2u texSize = sheetTexture->getSize();
    if (enemyType == Type::Tank) {
        sprite->setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(static_cast<int>(texSize.x), static_cast<int>(texSize.y))));
        return;
    }

    int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
    int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;

    int col = currentFrame % FRAME_COLS;
    int row = currentFrame / FRAME_COLS;

    sprite->setTextureRect(sf::IntRect({col * frameWidth, row * frameHeight}, {frameWidth, frameHeight}));
}

void Enemy::updateAnimation(float deltaTime) {
    if (enemyType == Type::Tank) return;

    animationTimer += deltaTime;

    if (animationTimer >= frameDuration) {
        animationTimer = 0.0f;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        updateSpriteRect();
    }
}

void Enemy::update(float deltaTime, int screenWidth, int screenHeight, const sf::Vector2f& playerPos,
                   std::vector<std::unique_ptr<Projectile>>& projectiles) {
    bool followingPath = (path != nullptr);
    updateMovement(deltaTime, screenWidth, screenHeight);

    if (!followingPath) {
        position += velocity * deltaTime;
    }

    if (sprite) {
        sprite->setPosition(position);
    }

    updateAnimation(deltaTime);

    if (shooter) {
        shooter->update(deltaTime, position, playerPos, projectiles);
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
