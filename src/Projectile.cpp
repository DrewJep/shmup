#include "Projectile.h"
#include <cmath>

// Static texture initialization
std::unique_ptr<sf::Texture> Projectile::texture = nullptr;

bool Projectile::loadTexture() {
    if (texture == nullptr) {
        texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile("assets/characters/shot.png")) {
            texture.reset();
            return false;
        }
    }
    return true;
}

void Projectile::unloadTexture() {
    texture.reset();
}

Projectile::Projectile(float x, float y, float angle, float speed)
    : position(x, y), speed(speed), currentFrame(0), 
      animationTimer(0.0f), frameDuration(0.05f), sprite(nullptr) { // 50ms per frame = 20 FPS animation
    // Calculate velocity based on angle (in radians)
    // Forward direction in isometric view is top-right (45 degrees or π/4 radians)
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
    
    // Ensure texture is loaded
    if (!texture) {
        loadTexture();
    }
    
    // Create sprite with texture
    if (texture) {
        sprite = std::make_unique<sf::Sprite>(*texture);
        
        // Calculate frame size from texture (assuming 2x3 grid)
        sf::Vector2u texSize = texture->getSize();
        int frameWidth = texSize.x / FRAME_COLS;
        int frameHeight = texSize.y / FRAME_ROWS;
        
        // Set the initial texture rect to first frame
        updateSpriteRect();
        
        // Set origin to center of frame
        sprite->setOrigin(sf::Vector2f(frameWidth / 2.0f, frameHeight / 2.0f));
        
        // No rotation needed - sprite is already oriented correctly (bottom-left to top-right)
        
        sprite->setPosition(position);
    }
}

void Projectile::updateSpriteRect() {
    if (!texture || !sprite) return;
    
    sf::Vector2u texSize = texture->getSize();
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
    if (sprite) {
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

