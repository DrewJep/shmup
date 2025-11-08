#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SFML/Graphics.hpp>
#include <memory>

class Projectile {
public:
    Projectile(float x, float y, float angle, float speed = 500.0f);
    
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const;
    bool isOffScreen(int screenWidth, int screenHeight) const;
    
    // Static texture management (shared across all projectiles)
    static bool loadTexture();
    static void unloadTexture();
    
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed;
    
    // Animation
    static std::unique_ptr<sf::Texture> texture;
    static const int FRAME_COLS = 2; // 2 columns in sprite sheet
    static const int FRAME_ROWS = 3; // 3 rows in sprite sheet
    static const int TOTAL_FRAMES = FRAME_COLS * FRAME_ROWS; // 6 frames total
    
    std::unique_ptr<sf::Sprite> sprite;
    int currentFrame;
    float animationTimer;
    float frameDuration; // Time per frame in seconds
    
    void updateAnimation(float deltaTime);
    void updateSpriteRect();
};

#endif // PROJECTILE_H

