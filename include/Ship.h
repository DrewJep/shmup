#ifndef SHIP_H
#define SHIP_H

#include <SFML/Graphics.hpp>
#include <memory>

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
    
    // Shooting
    bool shouldShoot(); // Returns true when ready to fire (call each frame)
    float getForwardAngle() const; // Get the angle the ship is facing (forward = top-right)
    // Health
    int getHealth() const;
    void takeDamage(int amount);
    sf::FloatRect getBounds() const;
    // Mode: Air or Ground (placeholder for later gameplay logic)
    enum class Mode { Air, Ground };
    Mode getMode() const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed;
    
    // Sprite representation
    sf::Texture texture;
    std::unique_ptr<sf::Sprite> sprite;
    
    // Health
    int health;
    
    // Helper to load texture
    bool loadTexture();
    
    // Input state tracking
    bool moveUp;
    bool moveDown;
    bool moveLeft;
    bool moveRight;
    bool shootPressed; // True when space key is held down
    
    // Shooting cooldown
    float fireRate; // Time between shots in seconds
    float timeSinceLastShot; // Accumulated time since last shot
    
    void updateMovement();
    void updateShooting(float deltaTime);
};

#endif // SHIP_H

