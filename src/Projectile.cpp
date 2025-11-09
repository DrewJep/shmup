#include "Projectile.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Static texture initialization
std::unique_ptr<sf::Texture> Projectile::texturePlayer = nullptr;
std::unique_ptr<sf::Texture> Projectile::textureEnemy = nullptr;

bool Projectile::loadTexture() {
    // Load player shot texture
    if (!texturePlayer) {
        texturePlayer = std::make_unique<sf::Texture>();
        if (!texturePlayer->loadFromFile("assets/characters/shot.png")) {
            texturePlayer.reset();
        }
    }

    // Load enemy (UFO) beam texture
    if (!textureEnemy) {
        textureEnemy = std::make_unique<sf::Texture>();
        if (!textureEnemy->loadFromFile("assets/characters/ufo_beam.png")) {
            // If specific enemy beam not found, fall back to player shot texture
            textureEnemy.reset();
        }
    }

    // At least one texture must be available
    return (texturePlayer != nullptr) || (textureEnemy != nullptr);
}

void Projectile::unloadTexture() {
    texturePlayer.reset();
    textureEnemy.reset();
}

Projectile::Projectile(float x, float y, float angle, float speed, Owner owner, float lifetimeIn, bool stretch, bool isPreview)
    : position(x, y), speed(speed), currentFrame(0), 
        animationTimer(0.0f), frameDuration(0.05f), sprite(nullptr), owner(owner),
        lifetime(lifetimeIn), stretchToLength(stretch), preview(isPreview) { // 50ms per frame = 20 FPS animation
    // Calculate velocity based on angle (in radians)
    // Forward direction in isometric view is top-right (45 degrees or π/4 radians)
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
    
    // Ensure texture is loaded
    
    loadTexture();
    
    // Create sprite with the appropriate texture for the owner
    sf::Texture* texPtr = nullptr;
    if (owner == Owner::Player && texturePlayer) texPtr = texturePlayer.get();
    if (owner == Owner::Enemy && textureEnemy) texPtr = textureEnemy.get();
    if (!texPtr && texturePlayer) texPtr = texturePlayer.get();

    // If this projectile is a beam (stretchToLength) we render it as a RectangleShape
    if (stretchToLength) {
        // Create a long rectangle for the beam instead of stretching a sprite. This avoids extreme sprite scaling.
        float beamLength = 2000.0f; // long enough to cross typical screens
        float thickness = preview ? 2.0f : 10.0f; // thin preview vs thicker beam

        beamShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(beamLength, thickness));
    beamShape->setOrigin(sf::Vector2f(0.0f, thickness / 2.0f)); // start at left-middle so beam originates at enemy
        beamShape->setPosition(position);

        // Rotation: use the provided angle (radians) converted to degrees
        float deg = angle * 180.0f / 3.14159265f;
    beamShape->setRotation(sf::degrees(deg));

        // Color/tint: preview is thin and semi-transparent
        if (preview) {
            beamShape->setFillColor(sf::Color(255, 40, 40, 140));
        } else {
            beamShape->setFillColor(sf::Color(255, 30, 30, 220));
        }
    } else if (texPtr) {
        // Regular sprite-based projectile
        sprite = std::make_unique<sf::Sprite>(*texPtr);

        // Calculate frame size from texture (assuming 2x3 grid)
        sf::Vector2u texSize = texPtr->getSize();
        int frameWidth = texSize.x / FRAME_COLS;
        int frameHeight = texSize.y / FRAME_ROWS;

        // Set the initial texture rect to first frame
        updateSpriteRect();

        // Origin center of frame for normal shots
        sprite->setOrigin(sf::Vector2f(frameWidth / 2.0f, frameHeight / 2.0f));

        // Rotate to align with travel direction. The art's nose points to top-right;
        // use a 135 degree offset so the forward direction aligns visually for enemy shots.
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
    int frameWidth = texSize.x / FRAME_COLS;
    int frameHeight = texSize.y / FRAME_ROWS;
    
    // Calculate which frame to show (currentFrame from 0 to TOTAL_FRAMES-1)
    int col = currentFrame % FRAME_COLS;
    int row = currentFrame / FRAME_COLS;
    
    // Set texture rectangle for current frame (SFML 3.0 uses position and size)
    sprite->setTextureRect(sf::IntRect(
        sf::Vector2i(col * frameWidth, row * frameHeight),
        sf::Vector2i(frameWidth, frameHeight)
    ));
}

void Projectile::updateAnimation(float deltaTime) {
    animationTimer += deltaTime;
    
    // Advance to next frame if enough time has passed
    if (animationTimer >= frameDuration) {
        animationTimer = 0.0f;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES; // Loop animation
        updateSpriteRect();
    }
}

void Projectile::update(float deltaTime) {
    position += velocity * deltaTime;
    // Update beam shape position if present (beams stay anchored at creation position but
    // still respect any non-zero velocity if used)
    if (beamShape) {
        beamShape->setPosition(position);
        // rotation stays as initialized (do not continuously rotate beams)
    }
    // Update sprite position/rotation for normal projectiles
    if (sprite) {
        if (owner == Owner::Enemy) {
            float travelRad = std::atan2(velocity.y, velocity.x);
            float deg = travelRad * 180.0f / 3.14159265f;
            sprite->setRotation(sf::degrees(deg - 135.0f));
        }
        sprite->setPosition(position);
    }
    updateAnimation(deltaTime);

    // Reduce lifetime if used (lifetime < 0 means unused)
    if (lifetime >= 0.0f) {
        lifetime -= deltaTime;
        if (lifetime < 0.0f) lifetime = 0.0f; // clamp to zero to mark expired
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
    // Manual intersection check for SFML 3.0
    // Two rectangles intersect if they overlap in both x and y axes
    bool xOverlap = (myBounds.position.x < otherBounds.position.x + otherBounds.size.x) &&
                    (otherBounds.position.x < myBounds.position.x + myBounds.size.x);
    bool yOverlap = (myBounds.position.y < otherBounds.position.y + otherBounds.size.y) &&
                    (otherBounds.position.y < myBounds.position.y + myBounds.size.y);
    return xOverlap && yOverlap;
}

bool Projectile::isOffScreen(int screenWidth, int screenHeight) const {
    // Check if projectile is off screen (with some margin)
    float margin = 50.0f;
    // If lifetime was specified (>=0) and has expired (==0), treat as off-screen
    if (lifetime >= 0.0f && lifetime == 0.0f) return true;
    return position.x < -margin || position.x > screenWidth + margin ||
           position.y < -margin || position.y > screenHeight + margin;
}

