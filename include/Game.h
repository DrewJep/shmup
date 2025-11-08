#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Ship.h"

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
    
    // Floor/Grid rendering
    void drawFloor(sf::RenderWindow& window);
    static const int FLOOR_GRID_SIZE = 20; // Grid cells across the floor
    
    // Timing
    sf::Clock clock;
    float deltaTime;
    
    // Game state
    bool isRunning;
};

#endif // GAME_H

