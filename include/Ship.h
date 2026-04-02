#ifndef SHIP_H
#define SHIP_H

#include <SFML/Graphics.hpp>
#include <memory>

#include "Actor.h"

class Ship : public Actor {
public:
    Ship(float x, float y, float speed = 300.0f);

    void update(float deltaTime);
    void handleInput(const sf::Keyboard::Key& key, bool isPressed);
    void updateInput(); // Call this each frame to process current input state
    void draw(sf::RenderWindow& window);

    void setPosition(float x, float y) override;

    float getSpeed() const;
    void setSpeed(float speed);

    // Shooting
    bool shouldShoot(); // Returns true when ready to fire (call each frame)
    float getForwardAngle() const; // Get the angle the ship is facing (forward = top-right)
    // Mode: Air or Ground (placeholder for later gameplay logic)
    enum class Mode { Air, Ground };
    Mode getMode() const;
    // Facing directions for ground (8-way)
    enum class Facing {
        Right,
        DownRight,
        Down,
        DownLeft,
        Left,
        UpLeft,
        Up,
        UpRight
    };

    // For ground mode controls: update facing via input (IJKL keys or mouse)
    void handleAimInput(const sf::Keyboard::Key& key, bool isPressed);
    void updateMouseAim(const sf::RenderWindow& window);
    void setFacingFromAngle(float angle);
    Facing getFacing() const;

    sf::FloatRect getBounds() const override;

private:
    float speed;

    // Sprite representation (textures owned by TextureCache)
    const sf::Texture* texAir = nullptr;
    std::unique_ptr<sf::Sprite> sprite;
    const sf::Texture* groundTexDownDiag = nullptr;
    const sf::Texture* groundTexStraight = nullptr;
    const sf::Texture* groundTexUpDiag = nullptr;

    // Ground-mode animation state
    int groundCurrentFrame;
    float groundAnimTimer;
    float groundFrameDuration;
    // Ground sprites are provided as 2 columns x 3 rows (each frame 32x32 in the assets)
    static const int GROUND_FRAME_COLS = 2;
    static const int GROUND_FRAME_ROWS = 3;
    static const int GROUND_TOTAL_FRAMES = GROUND_FRAME_COLS * GROUND_FRAME_ROWS;

    // Mode and facing
    Mode mode;
    Facing facing;
    // Aim keys state for twin-stick aiming (IJKL)
    bool aimUp;
    bool aimDown;
    bool aimLeft;
    bool aimRight;

    // Helper to load texture pointers from cache
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
