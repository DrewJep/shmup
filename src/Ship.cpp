#include "Ship.h"
#include <SFML/Graphics.hpp>
#include <cmath>

Ship::Ship(float x, float y, float speed)
    : position(x, y), velocity(0, 0), speed(speed), 
    moveUp(false), moveDown(false), moveLeft(false), moveRight(false),
    shootPressed(false), fireRate(0.15f), timeSinceLastShot(0.0f),
    sprite(nullptr), health(2) {
    // Load ship sprite texture
    if (loadTexture()) {
        // Create sprite with loaded texture
        sprite = std::make_unique<sf::Sprite>(texture);
        
        // Set origin to center of sprite for proper rotation and positioning
        sf::FloatRect bounds = sprite->getLocalBounds();
        sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f));
        
        sprite->setPosition(position);
    }
}

bool Ship::loadTexture() {
    return texture.loadFromFile("assets/characters/ship_nutral.png");
}

void Ship::update(float deltaTime) {
    // Update position based on velocity
    position += velocity * deltaTime;
    
    // Update shooting cooldown
    updateShooting(deltaTime);
    
    // Note: Bounds checking is handled by the Game class
    if (sprite) {
        sprite->setPosition(position);
    }
}

void Ship::handleInput(const sf::Keyboard::Key& key, bool isPressed) {
    switch (key) {
        case sf::Keyboard::Key::W:
        case sf::Keyboard::Key::Up:
            moveUp = isPressed;
            break;
        case sf::Keyboard::Key::S:
        case sf::Keyboard::Key::Down:
            moveDown = isPressed;
            break;
        case sf::Keyboard::Key::A:
        case sf::Keyboard::Key::Left:
            moveLeft = isPressed;
            break;
        case sf::Keyboard::Key::D:
        case sf::Keyboard::Key::Right:
            moveRight = isPressed;
            break;
        case sf::Keyboard::Key::Space:
            shootPressed = isPressed; // Track if space is held down
            break;
        default:
            break;
    }
}

void Ship::updateInput() {
    updateMovement();
}

void Ship::updateMovement() {
    // Reset velocity
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    
    // Apply movement based on input state
    if (moveUp) velocity.y -= speed;
    if (moveDown) velocity.y += speed;
    if (moveLeft) velocity.x -= speed;
    if (moveRight) velocity.x += speed;
    
    // Normalize diagonal movement to maintain consistent speed
    if (velocity.x != 0.0f && velocity.y != 0.0f) {
        float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        velocity.x = (velocity.x / length) * speed;
        velocity.y = (velocity.y / length) * speed;
    }
}

void Ship::draw(sf::RenderWindow& window) {
    if (sprite) {
        window.draw(*sprite);
    }
}

sf::Vector2f Ship::getPosition() const {
    return position;
}

void Ship::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    if (sprite) {
        sprite->setPosition(position);
    }
}

float Ship::getSpeed() const {
    return speed;
}

void Ship::setSpeed(float speed) {
    this->speed = speed;
}

bool Ship::shouldShoot() {
    // Can shoot if space is held and cooldown is ready
    if (shootPressed && timeSinceLastShot >= fireRate) {
        timeSinceLastShot = 0.0f; // Reset cooldown
        return true;
    }
    return false;
}

void Ship::updateShooting(float deltaTime) {
    // Accumulate time since last shot
    timeSinceLastShot += deltaTime;
}

float Ship::getForwardAngle() const {
    // Forward direction in isometric view is top-right (45 degrees = π/4 radians)
    // In screen coordinates, this is -45 degrees or 315 degrees
    // But we want it in radians, and top-right in screen space is approximately -π/4
    // Actually, in standard math coordinates, top-right is -45° = -π/4
    // But SFML's Y axis points down, so we need to adjust
    // For isometric forward (top-right), we want approximately -45 degrees
    return -3.14159f / 4.0f; // -45 degrees in radians (top-right direction)
}

int Ship::getHealth() const {
    return health;
}

void Ship::takeDamage(int amount) {
    health = std::max(0, health - amount);
}

sf::FloatRect Ship::getBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    return sf::FloatRect(position, sf::Vector2f(0.f, 0.f));
}

