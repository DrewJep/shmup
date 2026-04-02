#include "Projectile.h"
#include <cmath>
#include <iostream>

#include "TextureCache.h"

namespace {
constexpr const char* kPlayerShot = "assets/characters/shot.png";
constexpr const char* kEnemyBeam = "assets/characters/ufo_beam.png";
} // namespace

bool Projectile::loadTexture() {
    auto& c = TextureCache::instance();
    sf::Texture* a = c.load(kPlayerShot);
    sf::Texture* b = c.load(kEnemyBeam);
    return (a != nullptr) || (b != nullptr);
}

void Projectile::unloadTexture() {
    // Textures stay in TextureCache for reuse across systems.
}

Projectile::Projectile(float x, float y, float angle, float speed, Owner owner, float lifetimeIn, bool stretch, bool isPreview)
    : position(x, y),
      speed(speed),
      currentFrame(0),
      animationTimer(0.0f),
      frameDuration(0.05f),
      sprite(nullptr),
      owner(owner),
      lifetime(lifetimeIn),
      stretchToLength(stretch),
      preview(isPreview) {
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;

    loadTexture();
    auto& cache = TextureCache::instance();
    sf::Texture* texPlayer = cache.load(kPlayerShot);
    sf::Texture* texEnemy = cache.load(kEnemyBeam);

    sf::Texture* texPtr = nullptr;
    if (owner == Owner::Player && texPlayer) texPtr = texPlayer;
    if (owner == Owner::Enemy && texEnemy) texPtr = texEnemy;
    if (!texPtr && texPlayer) texPtr = texPlayer;

    if (stretchToLength) {
        float beamLength = 2000.0f;
        float thickness = preview ? 2.0f : 10.0f;

        beamShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(beamLength, thickness));
        beamShape->setOrigin(sf::Vector2f(0.0f, thickness / 2.0f));
        beamShape->setPosition(position);

        float deg = angle * 180.0f / 3.14159265f;
        beamShape->setRotation(sf::degrees(deg));

        if (preview) {
            beamShape->setFillColor(sf::Color(255, 40, 40, 140));
        } else {
            beamShape->setFillColor(sf::Color(255, 30, 30, 220));
        }
    } else if (texPtr) {
        sprite = std::make_unique<sf::Sprite>(*texPtr);

        sf::Vector2u texSize = texPtr->getSize();
        int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
        int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;

        updateSpriteRect();

        sprite->setOrigin(sf::Vector2f(frameWidth / 2.0f, frameHeight / 2.0f));

        if (owner == Owner::Enemy) {
            float travelRad = std::atan2(velocity.y, velocity.x);
            float deg = travelRad * 180.0f / 3.14159265f;
            sprite->setRotation(sf::degrees(deg - 135.0f));
        }

        sprite->setPosition(position);
    }
}

Projectile::Owner Projectile::getOwner() const { return owner; }

void Projectile::updateSpriteRect() {
    if (!sprite) return;

    const sf::Texture& tex = sprite->getTexture();
    sf::Vector2u texSize = tex.getSize();
    int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
    int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;

    int col = currentFrame % FRAME_COLS;
    int row = currentFrame / FRAME_COLS;

    sprite->setTextureRect(sf::IntRect(sf::Vector2i(col * frameWidth, row * frameHeight),
                                       sf::Vector2i(frameWidth, frameHeight)));
}

void Projectile::updateAnimation(float deltaTime) {
    animationTimer += deltaTime;

    if (animationTimer >= frameDuration) {
        animationTimer = 0.0f;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        updateSpriteRect();
    }
}

void Projectile::update(float deltaTime) {
    position += velocity * deltaTime;
    if (beamShape) {
        beamShape->setPosition(position);
    }
    if (sprite) {
        if (owner == Owner::Enemy) {
            float travelRad = std::atan2(velocity.y, velocity.x);
            float deg = travelRad * 180.0f / 3.14159265f;
            sprite->setRotation(sf::degrees(deg - 135.0f));
        }
        sprite->setPosition(position);
    }
    updateAnimation(deltaTime);

    if (lifetime >= 0.0f) {
        lifetime -= deltaTime;
        if (lifetime < 0.0f) lifetime = 0.0f;
    }
}

void Projectile::draw(sf::RenderWindow& window) {
    if (beamShape) {
        window.draw(*beamShape);
        return;
    }
    if (sprite) {
        window.draw(*sprite);
    }
}

sf::Vector2f Projectile::getPosition() const {
    return position;
}

sf::FloatRect Projectile::getBounds() const {
    if (beamShape) {
        return beamShape->getGlobalBounds();
    }
    if (sprite) {
        return sprite->getGlobalBounds();
    }
    return sf::FloatRect(sf::Vector2f(position.x, position.y), sf::Vector2f(0, 0));
}

bool Projectile::checkCollision(const sf::FloatRect& otherBounds) const {
    sf::FloatRect myBounds = getBounds();
    bool xOverlap = (myBounds.position.x < otherBounds.position.x + otherBounds.size.x) &&
                    (otherBounds.position.x < myBounds.position.x + myBounds.size.x);
    bool yOverlap = (myBounds.position.y < otherBounds.position.y + otherBounds.size.y) &&
                    (otherBounds.position.y < myBounds.position.y + myBounds.size.y);
    return xOverlap && yOverlap;
}

bool Projectile::isOffScreen(int screenWidth, int screenHeight) const {
    float margin = 50.0f;
    if (lifetime >= 0.0f && lifetime == 0.0f) return true;
    return position.x < -margin || position.x > screenWidth + margin || position.y < -margin ||
           position.y > screenHeight + margin;
}
