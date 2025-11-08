#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <memory>
#include "Path.h"

class Enemy {
public:
    Enemy(float x, float y, float speed = 100.0f);
    
    void update(float deltaTime, int screenWidth, int screenHeight);
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;
    
    int getHealth() const;
    void takeDamage(int damage);
    bool isDead() const;
    // Path movement
    void setPath(std::unique_ptr<Path> p);
    bool hasPath() const;
    
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed;
    int health;
    int maxHealth;
    
    // Animation
    static std::unique_ptr<sf::Texture> texture;
    static const int FRAME_COLS = 2;
    static const int FRAME_ROWS = 3;
    static const int TOTAL_FRAMES = FRAME_COLS * FRAME_ROWS;

    std::unique_ptr<sf::Sprite> sprite;
    int currentFrame;
    float animationTimer;
    float frameDuration;

    // Movement pattern
    float movementTimer;
    float directionChangeInterval;
    std::unique_ptr<Path> path;
    
    // Internal helpers
    void updateAnimation(float deltaTime);
    void updateSpriteRect();
    void updateMovement(float deltaTime, int screenWidth, int screenHeight);
    static bool loadTexture();
};

#endif // ENEMY_H
