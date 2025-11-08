#include "Ship.h"
#include <SFML/Graphics.hpp>
#include <cmath>

Ship::Ship(float x, float y, float speed)
    : position(x, y), velocity(0, 0), speed(speed), 
      moveUp(false), moveDown(false), moveLeft(false), moveRight(false) {
    // Create a simple triangle shape to represent the ship
    // This will be replaced with a sprite later
    // Points top-right (isometric forward direction)
    shape.setPointCount(3);
    shape.setPoint(0, sf::Vector2f(15, -15));  // Tip pointing top-right
    shape.setPoint(1, sf::Vector2f(-10, 10));  // Bottom-left base
    shape.setPoint(2, sf::Vector2f(5, 10));    // Bottom-right base
    
    shape.setFillColor(sf::Color::Cyan);
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(2.0f);
    shape.setPosition(position);
}

void Ship::update(float deltaTime) {
    // Update position based on velocity
    position += velocity * deltaTime;
    
    // Note: Bounds checking is handled by the Game class
    shape.setPosition(position);
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
    window.draw(shape);
}

sf::Vector2f Ship::getPosition() const {
    return position;
}

void Ship::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    shape.setPosition(position);
}

float Ship::getSpeed() const {
    return speed;
}

void Ship::setSpeed(float speed) {
    this->speed = speed;
}

