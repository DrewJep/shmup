#include "Enemy.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "ShootingPattern.h"
// Path is included via Enemy.h

// Static texture
std::unique_ptr<sf::Texture> Enemy::texture = nullptr;
std::unique_ptr<sf::Texture> Enemy::tankTexture = nullptr;

bool Enemy::loadTexture() {
    if (!texture) {
        texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile("assets/characters/ufo.png")) {
            texture.reset();
            return false;
        }
    }
    return true;
}

// Load tank texture (optional); returns true if available
bool Enemy::loadTankTexture() {
    if (!tankTexture) {
        tankTexture = std::make_unique<sf::Texture>();
        if (!tankTexture->loadFromFile("assets/characters/tank.png")) {
            tankTexture.reset();
            return false;
        }
    }
    return true;
}

Enemy::Enemy(float x, float y, float speed, Type type)
    : position(x, y), speed(speed), health(1), maxHealth(1),
    movementTimer(0.0f), directionChangeInterval(1.0f + (std::rand() % 200) / 100.0f),
      currentFrame(0), animationTimer(0.0f), frameDuration(0.08f), sprite(nullptr), enemyType(type)
{
    // Load appropriate texture based on enemy type
    if (enemyType == Type::Tank) {
        if (loadTankTexture() && tankTexture) {
            sprite = std::make_unique<sf::Sprite>(*tankTexture);
            // Tank image expected to be a single-frame sprite. Use full texture rect.
            if (sprite) {
                sprite->setOrigin(sf::Vector2f(tankTexture->getSize().x / 2.f, tankTexture->getSize().y / 2.f));
                sprite->setPosition(position);
            }
        } else {
            // Fallback to UFO texture if tank not available
            loadTexture();
            if (texture) {
                sprite = std::make_unique<sf::Sprite>(*texture);
                updateSpriteRect();
                sf::Vector2u texSize = texture->getSize();
                int frameWidth = texSize.x / FRAME_COLS;
                int frameHeight = texSize.y / FRAME_ROWS;
                sprite->setOrigin({frameWidth / 2.f, frameHeight / 2.f});
                sprite->setPosition(position);
            }
        }
    } else {
        loadTexture();
        if (texture) {
            sprite = std::make_unique<sf::Sprite>(*texture);

            // Set initial animation frame
            updateSpriteRect();

            // Set origin to center of frame
            sf::Vector2u texSize = texture->getSize();
            int frameWidth = texSize.x / FRAME_COLS;
            int frameHeight = texSize.y / FRAME_ROWS;
            sprite->setOrigin({frameWidth / 2.f, frameHeight / 2.f});

            sprite->setPosition(position);
        }
    }

    // Start enemy in random direction
    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
}
    void Enemy::setHealth(int h) {
        health = h;
        maxHealth = std::max(1, h);
    }

void Enemy::updateSpriteRect() {
    // If this enemy uses the UFO sprite sheet, pick a sub-rect; tanks use full texture
    sf::Texture* texPtr = nullptr;
    if (enemyType == Type::Tank) texPtr = tankTexture.get();
    else texPtr = texture.get();
    if (!texPtr || !sprite) return;

    sf::Vector2u texSize = texPtr->getSize();
    // If texture appears to be a sheet (width >= FRAME_COLS*height/...), try to slice
    if (enemyType == Type::Tank) {
        // single-frame: full texture
        sprite->setTextureRect(sf::IntRect({0,0}, sf::Vector2i(texSize.x, texSize.y)));
        return;
    }

    int frameWidth = texSize.x / FRAME_COLS;
    int frameHeight = texSize.y / FRAME_ROWS;

    int col = currentFrame % FRAME_COLS;
    int row = currentFrame / FRAME_COLS;

    sprite->setTextureRect(sf::IntRect(
        {col * frameWidth, row * frameHeight},
        {frameWidth, frameHeight}
    ));
}

void Enemy::updateAnimation(float deltaTime) {
    animationTimer += deltaTime;

    if (animationTimer >= frameDuration) {
        animationTimer = 0.0f;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        updateSpriteRect();
    }
}

void Enemy::update(float deltaTime, int screenWidth, int screenHeight, const sf::Vector2f& playerPos, std::vector<std::unique_ptr<Projectile>>& projectiles) {
    // If following a path, updateMovement will set position directly.
    bool followingPath = (path != nullptr);
    updateMovement(deltaTime, screenWidth, screenHeight);

    // Only apply velocity-based movement when not following a path
    if (!followingPath) {
        position += velocity * deltaTime;
    }

    if (sprite) {
        sprite->setPosition(position);
    }

    // Animate
    updateAnimation(deltaTime);

    // Allow shooter to spawn projectiles
    if (shooter) {
        shooter->update(deltaTime, position, playerPos, projectiles);
    }
}

void Enemy::updateMovement(float deltaTime, int screenWidth, int screenHeight) {
    // If a path is set, let it control position
    if (path) {
        path->update(deltaTime);
        position = path->getPosition();
        // Optionally update velocity for visual smoothing
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

    // float margin = 50.0f;
    // if (position.x < screenWidth * 0.4f) velocity.x = std::abs(velocity.x);
    // if (position.x > screenWidth - margin) velocity.x = -std::abs(velocity.x);
    // if (position.y < margin) velocity.y = std::abs(velocity.y);
    // if (position.y > screenHeight - margin) velocity.y = -std::abs(velocity.y);
}

void Enemy::setPath(std::unique_ptr<Path> p) {
    path = std::move(p);
    if (path) {
        // Ensure path starts from current position unless the path already starts elsewhere
        path->setStart(position);
        // When following a path, zero movement velocity so we don't add it each frame
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

sf::Vector2f Enemy::getPosition() const { return position; }

sf::FloatRect Enemy::getBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    return sf::FloatRect({position.x, position.y}, {0.f, 0.f});
}

int Enemy::getHealth() const { return health; }

void Enemy::takeDamage(int damage) { health = std::max(0, health - damage); }

bool Enemy::isDead() const { return health <= 0; }
