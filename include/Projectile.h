#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SFML/Graphics.hpp>
#include <memory>

class Projectile {
public:
    enum class Owner { Player, Enemy };

    // lifetime: seconds before auto-destroy (negative = use off-screen test)
    // stretchToLength: if true, sprite will be stretched along X to a large length (useful for beams)
    // preview: if true, rendering will be thin/semi-transparent to indicate a warning
    Projectile(float x, float y, float angle, float speed = 500.0f, Owner owner = Owner::Player,
               float lifetime = -1.0f, bool stretchToLength = false, bool preview = false);
    
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const;
    bool isOffScreen(int screenWidth, int screenHeight) const;
    sf::FloatRect getBounds() const;
    bool checkCollision(const sf::FloatRect& otherBounds) const;
    Owner getOwner() const;
    
    // Static texture management (shared across all projectiles)
    static bool loadTexture();
    static void unloadTexture();
    
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed;
    
    // Animation textures (separate for player and enemy shots)
    static std::unique_ptr<sf::Texture> texturePlayer;
    static std::unique_ptr<sf::Texture> textureEnemy;
    static const int FRAME_COLS = 2; // 2 columns in sprite sheet
    static const int FRAME_ROWS = 3; // 3 rows in sprite sheet
    static const int TOTAL_FRAMES = FRAME_COLS * FRAME_ROWS; // 6 frames total
    
    std::unique_ptr<sf::Sprite> sprite;
    int currentFrame;
    float animationTimer;
    float frameDuration; // Time per frame in seconds
    Owner owner;
    // For beams / preview visuals
    float lifetime; // seconds remaining; negative = not used
    bool stretchToLength;
    bool preview;
    
    void updateAnimation(float deltaTime);
    void updateSpriteRect();
};

#endif // PROJECTILE_H

