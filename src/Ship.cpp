#include "Ship.h"
#include <SFML/Graphics.hpp>
#include <cmath>

#include <iostream>

#include "IsometricUtils.h"

Ship::Ship(float x, float y, float speed)
    : position(x, y), velocity(0, 0), speed(speed), 
    moveUp(false), moveDown(false), moveLeft(false), moveRight(false),
    shootPressed(false), fireRate(0.15f), timeSinceLastShot(0.0f),
    sprite(nullptr), health(20) {
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

Ship::Mode Ship::getMode() const {
    // Placeholder: always return Air for now. Later gameplay can change this.
    return Mode::Air;
}

bool Ship::loadTexture() {
    // Prefer new player sprite if present, fall back to the older character sprite
    const std::vector<std::string> candidates = {
        "assets/characters/player/player_sky.png"
    };

    for (const auto &p : candidates) {
        if (texture.loadFromFile(p)) {
            std::cout << "Loaded ship texture: " << p << std::endl;
            return true;
        }
    }
    return false;
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
    // Compute forward angle that matches the isometric projection so "forward"
    // (one tile up in world coordinates) appears visually as top-right on screen.
    // world vector (0, -1) maps to screen delta: (TILE_WIDTH/2, -TILE_HEIGHT/2)
    // So angle = atan2(dy, dx) = atan2(-TILE_HEIGHT/2, TILE_WIDTH/2) = -atan2(TILE_HEIGHT, TILE_WIDTH)
    return -std::atan2(IsometricUtils::TILE_HEIGHT, IsometricUtils::TILE_WIDTH);
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

