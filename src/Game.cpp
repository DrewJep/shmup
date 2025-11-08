#include "Game.h"
#include "IsometricUtils.h"
#include "Projectile.h"
#include "Path.h"
#include <iostream>
#include <optional>
#include <cmath>

const std::string Game::WINDOW_TITLE = "Isometric Shmup";

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), WINDOW_TITLE),
      playerShip(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 300.0f),
      deltaTime(0.0f),
      isRunning(true) {
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    
    // Pre-load projectile texture
    Projectile::loadTexture();
    
    // Spawn 3 enemies on the right side of the screen that trail each other along a patrol path
    float enemyX = WINDOW_WIDTH * 0.85f;
    float enemyY = WINDOW_HEIGHT / 2.0f;

    // Wider patrol that travels across more of the screen in a smooth loop
    std::vector<sf::Vector2f> patrol = {
        { WINDOW_WIDTH * 0.85f, WINDOW_HEIGHT * 0.50f },
        { WINDOW_WIDTH * 0.60f, WINDOW_HEIGHT * 0.25f },
        { WINDOW_WIDTH * 0.30f, WINDOW_HEIGHT * 0.50f },
        { WINDOW_WIDTH * 0.60f, WINDOW_HEIGHT * 0.75f }
    };

    // Create three enemies staggered behind each other along the path
    const int enemyCount = 3;
    const float spacing = 40.0f; // pixels to stagger spawn positions
    for (int i = 0; i < enemyCount; ++i) {
        float spawnX = enemyX - i * spacing;
        float spawnY = enemyY;
        enemies.push_back(std::make_unique<Enemy>(spawnX, spawnY, 80.0f));

        // Each enemy gets its own Path instance so internal position advances separately.
        auto p = std::make_unique<Path>(patrol, 80.0f, true);
        enemies.back()->setPath(std::move(p));
    }
}

Game::~Game() {
    // Cleanup if needed
}

void Game::run() {
    while (isRunning && window.isOpen()) {
        deltaTime = clock.restart().asSeconds();
        
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    while (std::optional<sf::Event> event = window.pollEvent()) {
        // Handle window closed event
        if (event->is<sf::Event::Closed>()) {
            window.close();
            isRunning = false;
        }
        
        // Handle key press events
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            playerShip.handleInput(keyPressed->code, true);
            
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                window.close();
                isRunning = false;
            }
        }
        
        // Handle key release events
        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            playerShip.handleInput(keyReleased->code, false);
        }
    }
}

void Game::update(float deltaTime) {
    // Update input state
    playerShip.updateInput();
    
    // Handle shooting (call shouldShoot each frame - it handles cooldown internally)
    if (playerShip.shouldShoot()) {
        sf::Vector2f shipPos = playerShip.getPosition();
        float angle = playerShip.getForwardAngle();
        
        // Spawn projectile slightly forward so it doesn't overlap with ship
        // Offset by ~30 pixels in the forward direction
        float offsetDistance = 30.0f;
        float spawnX = shipPos.x + std::cos(angle) * offsetDistance;
        float spawnY = shipPos.y + std::sin(angle) * offsetDistance;
        
        projectiles.push_back(std::make_unique<Projectile>(spawnX, spawnY, angle));
    }
    
    // Update game objects
    playerShip.update(deltaTime);
    
    // Update projectiles
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        (*it)->update(deltaTime);
        
        // Remove projectiles that are off screen
        if ((*it)->isOffScreen(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update enemies
    for (auto it = enemies.begin(); it != enemies.end();) {
        (*it)->update(deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT);
        
        // Remove dead enemies
        if ((*it)->isDead()) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
    
    // Check collisions between projectiles and enemies
    checkCollisions();
    
    // Keep ship within screen bounds
    sf::Vector2f pos = playerShip.getPosition();
    float shipRadius = 15.0f;
    
    if (pos.x < shipRadius) playerShip.setPosition(shipRadius, pos.y);
    if (pos.x > WINDOW_WIDTH - shipRadius) playerShip.setPosition(WINDOW_WIDTH - shipRadius, pos.y);
    if (pos.y < shipRadius) playerShip.setPosition(pos.x, shipRadius);
    if (pos.y > WINDOW_HEIGHT - shipRadius) playerShip.setPosition(pos.x, WINDOW_HEIGHT - shipRadius);
}

void Game::render() {
    // Clear with a dark background (space-like)
    window.clear(sf::Color(20, 20, 40));
    
    // Draw floor first (so objects appear on top)
    drawFloor(window);
    
    // Draw projectiles first (so ship appears on top)
    for (const auto& projectile : projectiles) {
        projectile->draw(window);
    }
    
    // Draw enemies
    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }
    
    // Draw ship on top
    playerShip.draw(window);
    
    // Display everything
    window.display();
}

void Game::drawFloor(sf::RenderWindow& window) {
    // Draw an isometric floor grid
    // We'll create a diamond/tile pattern using the isometric projection
    
    sf::VertexArray gridLines(sf::PrimitiveType::Lines);
    sf::Color gridColor(60, 60, 80, 180); // Semi-transparent grid
    
    // Draw grid lines in isometric space using tile units (not pixel-multiplied coordinates)
    // We need to cover the visible area with isometric tiles
    int gridWidth = FLOOR_GRID_SIZE;
    int gridHeight = FLOOR_GRID_SIZE;

    // Offset to center the grid
    float offsetX = WINDOW_WIDTH / 2.0f;
    float offsetY = WINDOW_HEIGHT / 3.0f; // Position floor in lower portion of screen

    // Draw vertical lines (constant worldX, varying worldY)
    for (int i = -gridWidth; i <= gridWidth; ++i) {
        float worldX = static_cast<float>(i); // tile units

        // Top point (tile units)
        float worldY1 = static_cast<float>(-gridHeight);
        sf::Vector2f screen1 = IsometricUtils::worldToScreen(worldX, worldY1);
        screen1.x += offsetX;
        screen1.y += offsetY;

        // Bottom point (tile units)
        float worldY2 = static_cast<float>(gridHeight);
        sf::Vector2f screen2 = IsometricUtils::worldToScreen(worldX, worldY2);
        screen2.x += offsetX;
        screen2.y += offsetY;

        sf::Vertex v1;
        v1.position = screen1;
        v1.color = gridColor;
        sf::Vertex v2;
        v2.position = screen2;
        v2.color = gridColor;
        gridLines.append(v1);
        gridLines.append(v2);
    }

    // Draw horizontal lines (constant worldY, varying worldX)
    for (int i = -gridHeight; i <= gridHeight; ++i) {
        float worldY = static_cast<float>(i); // tile units

        // Left point
        float worldX1 = static_cast<float>(-gridWidth);
        sf::Vector2f screen1 = IsometricUtils::worldToScreen(worldX1, worldY);
        screen1.x += offsetX;
        screen1.y += offsetY;

        // Right point
        float worldX2 = static_cast<float>(gridWidth);
        sf::Vector2f screen2 = IsometricUtils::worldToScreen(worldX2, worldY);
        screen2.x += offsetX;
        screen2.y += offsetY;

        sf::Vertex v1;
        v1.position = screen1;
        v1.color = gridColor;
        sf::Vertex v2;
        v2.position = screen2;
        v2.color = gridColor;
        gridLines.append(v1);
        gridLines.append(v2);
    }
    
    window.draw(gridLines);
    
    // Draw some floor tiles for better visual effect
    sf::Color tileColor1(40, 50, 60, 200);
    sf::Color tileColor2(50, 60, 70, 200);
    
    for (int x = -gridWidth / 2; x < gridWidth / 2; ++x) {
        for (int y = -gridHeight / 2; y < gridHeight / 2; ++y) {
            // Use tile-unit coordinates (x,y) rather than multiplying by TILE_WIDTH/TILE_HEIGHT
            float worldX = static_cast<float>(x);
            float worldY = static_cast<float>(y);
            
            // Create a diamond shape for each tile
            sf::ConvexShape tile;
            tile.setPointCount(4);
            
            // Calculate the four corners of the isometric tile
            sf::Vector2f center = IsometricUtils::worldToScreen(worldX, worldY);
            center.x += offsetX;
            center.y += offsetY;
            
            // Top corner (half tile up)
            sf::Vector2f top = IsometricUtils::worldToScreen(worldX, worldY - 0.5f);
            top.x += offsetX;
            top.y += offsetY;
            
            // Right corner (half tile right)
            sf::Vector2f right = IsometricUtils::worldToScreen(worldX + 0.5f, worldY);
            right.x += offsetX;
            right.y += offsetY;
            
            // Bottom corner (half tile down)
            sf::Vector2f bottom = IsometricUtils::worldToScreen(worldX, worldY + 0.5f);
            bottom.x += offsetX;
            bottom.y += offsetY;
            
            // Left corner (half tile left)
            sf::Vector2f left = IsometricUtils::worldToScreen(worldX - 0.5f, worldY);
            left.x += offsetX;
            left.y += offsetY;
            
            tile.setPoint(0, top - center);
            tile.setPoint(1, right - center);
            tile.setPoint(2, bottom - center);
            tile.setPoint(3, left - center);
            tile.setPosition(center);
            tile.setFillColor((x + y) % 2 == 0 ? tileColor1 : tileColor2);
            tile.setOutlineColor(sf::Color(70, 80, 90, 150));
            tile.setOutlineThickness(1.0f);
            
            window.draw(tile);
        }
    }
}

void Game::checkCollisions() {
    // Check each projectile against each enemy
    for (auto projIt = projectiles.begin(); projIt != projectiles.end();) {
        bool projectileHit = false;
        
        for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
            if ((*projIt)->checkCollision((*enemyIt)->getBounds())) {
                // Projectile hit enemy
                (*enemyIt)->takeDamage(1); // Beams do 1 damage
                projectileHit = true;
                
                // If enemy is dead, it will be removed in the update loop
                break;
            } else {
                ++enemyIt;
            }
        }
        
        // Remove projectile if it hit an enemy
        if (projectileHit) {
            projIt = projectiles.erase(projIt);
        } else {
            ++projIt;
        }
    }
}

