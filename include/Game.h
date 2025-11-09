#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include "Ship.h"
#include "Projectile.h"
#include "Enemy.h"

class Game {
public:
    Game();
    ~Game();
    
    void run();
    
private:
    void processEvents();
    void update(float deltaTime);
    void render();
    
    // Window
    sf::RenderWindow window;
    // Size of the in-game play area (classic 16-bit feel). Use SNES-like resolution.
    // We'll default to 320x224 (width x height).
    static const int PLAY_WIDTH = 320;
    static const int PLAY_HEIGHT = 224;
    // Make the window an integer scale of the retro play area so borders are small.
    // Default to 2x scale of 320x224 -> 640x448 which is close to classic CRT sizes.
    static const int WINDOW_WIDTH = PLAY_WIDTH * 2; // 640
    static const int WINDOW_HEIGHT = PLAY_HEIGHT * 2; // 448
    static const std::string WINDOW_TITLE;
    
    // Game objects
    Ship playerShip;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<Enemy>> enemies;
    
    // Collision detection
    void checkCollisions();
    
    // Floor/Grid rendering
    void drawFloor(sf::RenderWindow& window);
    static const int FLOOR_GRID_SIZE = 20; // Grid cells across the floor
    float backgroundScrollX; // Offset for scrolling background (wraps between 0 and TILE_WIDTH)
    float backgroundScrollY; // Offset for scrolling background (wraps between 0 and TILE_HEIGHT)
    static constexpr float SCROLL_SPEED = 240.0f; // Pixels per second for background scroll
    
    // Timing
    sf::Clock clock;
    float deltaTime;
    float elapsedTime; // seconds since game start

    // UI
    sf::Font uiFont;
    bool uiHasFont;
    // Music
    sf::Music backgroundMusic;
    bool musicLoaded;
    
    // Game state
    bool isRunning;
    int currentLevel;
};

#endif // GAME_H

