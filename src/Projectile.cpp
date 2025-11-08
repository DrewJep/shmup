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

    if (texPtr) {
        sprite = std::make_unique<sf::Sprite>(*texPtr);

        // Calculate frame size from texture (assuming 2x3 grid)
        sf::Vector2u texSize = texPtr->getSize();
        int frameWidth = texSize.x / FRAME_COLS;
        int frameHeight = texSize.y / FRAME_ROWS;

        // Set the initial texture rect to first frame
        updateSpriteRect();

        // Default origin is center of frame for normal shots, but if we're stretching
        // the sprite to make a beam, set the origin at the left-middle so the beam
        // extends outward from the enemy position (tail at origin).
        if (stretchToLength) {
            sprite->setOrigin(sf::Vector2f(0.0f, frameHeight / 2.0f));
        } else {
            sprite->setOrigin(sf::Vector2f(frameWidth / 2.0f, frameHeight / 2.0f));
        }

        // If requested, stretch the sprite along X so it looks like a beam reaching far
        if (stretchToLength) {
            // Choose a long length but cap the scale to avoid extreme values that may crash GPU drivers
            float targetLength = 1200.0f;
            float rawScaleX = targetLength / static_cast<float>(frameWidth);
            float maxScaleX = 20.0f; // avoid insane scaling
            float scaleX = std::min(rawScaleX, maxScaleX);
            float scaleY = preview ? 0.25f : 1.0f;
            sprite->setScale(sf::Vector2f(scaleX, scaleY));
            if (preview) {
                sprite->setColor(sf::Color(255, 80, 80, 160));
            } else {
                sprite->setColor(sf::Color(255, 180, 60, 220));
            }
            // Debug info for beams
            std::cout << "Projectile: created " << (preview ? "preview" : "beam") << " frameW=" << frameWidth
                      << " scaleX=" << scaleX << " scaleY=" << scaleY << " lifetime=" << lifetime << std::endl;
        }

        // Rotate to align with travel direction. The art's nose points to top-right;
        // use a 135 degree offset so the forward direction aligns visually.
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
    // Update sprite position and orientation each frame in case velocity changes
    if (sprite) {
        // Recompute rotation from current velocity so sprite always faces travel direction
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
    if (sprite) {
        window.draw(*sprite);
    }
}

sf::Vector2f Projectile::getPosition() const {
    return position;
}

sf::FloatRect Projectile::getBounds() const {
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

