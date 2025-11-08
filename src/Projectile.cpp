#include "Projectile.h"
#include <cmath>

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

Projectile::Projectile(float x, float y, float angle, float speed, Owner owner)
        : position(x, y), speed(speed), currentFrame(0), 
            animationTimer(0.0f), frameDuration(0.05f), sprite(nullptr), owner(owner) { // 50ms per frame = 20 FPS animation
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
        
        // Set origin to center of frame
        sprite->setOrigin(sf::Vector2f(frameWidth / 2.0f, frameHeight / 2.0f));
        
        // Rotate enemy projectile sprite so its "front" (top-right in the PNG) points along velocity.
        // Compute actual travel direction from velocity to avoid mismatch between passed-in angle and
        // the final velocity (safer if velocity is modified elsewhere). Subtract 45 degrees because
        // the artwork's forward direction points to the top-right in the texture.
        if (owner == Owner::Enemy) {
            float travelRad = std::atan2(velocity.y, velocity.x);
            float deg = travelRad * 180.0f / 3.14159265f;
            sprite->setRotation(sf::degrees(deg - 45.0f));
        }

        // Position the sprite at the projectile position
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
    return position.x < -margin || position.x > screenWidth + margin ||
           position.y < -margin || position.y > screenHeight + margin;
}

