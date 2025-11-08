#ifndef SHIP_H
#define SHIP_H

#include <SFML/Graphics.hpp>

class Ship {
public:
    Ship(float x, float y, float speed = 300.0f);
    
    void update(float deltaTime);
    void handleInput(const sf::Keyboard::Key& key, bool isPressed);
    void updateInput(); // Call this each frame to process current input state
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const;
    void setPosition(float x, float y);
    
    float getSpeed() const;
    void setSpeed(float speed);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed;
    
    // Simple shape representation (will be replaced with sprite later)
    sf::ConvexShape shape;
    
    // Input state tracking
    bool moveUp;
    bool moveDown;
    bool moveLeft;
    bool moveRight;
    
    void updateMovement();
};

#endif // SHIP_H

