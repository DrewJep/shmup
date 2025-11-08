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
    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;
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
};

#endif // GAME_H

