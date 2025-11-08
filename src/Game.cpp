#include "Game.h"
#include "IsometricUtils.h"
#include "Projectile.h"
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
    
    // Draw grid lines in isometric space
    // We need to cover the visible area with isometric tiles
    int gridWidth = FLOOR_GRID_SIZE;
    int gridHeight = FLOOR_GRID_SIZE;
    
    // Offset to center the grid
    float offsetX = WINDOW_WIDTH / 2.0f;
    float offsetY = WINDOW_HEIGHT / 3.0f; // Position floor in lower portion of screen
    
    // Draw vertical lines (going from top-left to bottom-right in isometric view)
    for (int i = -gridWidth; i <= gridWidth; ++i) {
        float worldX = i * IsometricUtils::TILE_WIDTH;
        
        // Top point
        float worldY1 = -gridHeight * IsometricUtils::TILE_HEIGHT;
        sf::Vector2f screen1 = IsometricUtils::worldToScreen(worldX, worldY1);
        screen1.x += offsetX;
        screen1.y += offsetY;
        
        // Bottom point
        float worldY2 = gridHeight * IsometricUtils::TILE_HEIGHT;
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
    
    // Draw horizontal lines (going from top-right to bottom-left in isometric view)
    for (int i = -gridHeight; i <= gridHeight; ++i) {
        float worldY = i * IsometricUtils::TILE_HEIGHT;
        
        // Left point
        float worldX1 = -gridWidth * IsometricUtils::TILE_WIDTH;
        sf::Vector2f screen1 = IsometricUtils::worldToScreen(worldX1, worldY);
        screen1.x += offsetX;
        screen1.y += offsetY;
        
        // Right point
        float worldX2 = gridWidth * IsometricUtils::TILE_WIDTH;
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
            float worldX = x * IsometricUtils::TILE_WIDTH;
            float worldY = y * IsometricUtils::TILE_HEIGHT;
            
            // Create a diamond shape for each tile
            sf::ConvexShape tile;
            tile.setPointCount(4);
            
            // Calculate the four corners of the isometric tile
            sf::Vector2f center = IsometricUtils::worldToScreen(worldX, worldY);
            center.x += offsetX;
            center.y += offsetY;
            
            // Top corner
            sf::Vector2f top = IsometricUtils::worldToScreen(worldX, worldY - IsometricUtils::TILE_HEIGHT / 2.0f);
            top.x += offsetX;
            top.y += offsetY;
            
            // Right corner
            sf::Vector2f right = IsometricUtils::worldToScreen(worldX + IsometricUtils::TILE_WIDTH / 2.0f, worldY);
            right.x += offsetX;
            right.y += offsetY;
            
            // Bottom corner
            sf::Vector2f bottom = IsometricUtils::worldToScreen(worldX, worldY + IsometricUtils::TILE_HEIGHT / 2.0f);
            bottom.x += offsetX;
            bottom.y += offsetY;
            
            // Left corner
            sf::Vector2f left = IsometricUtils::worldToScreen(worldX - IsometricUtils::TILE_WIDTH / 2.0f, worldY);
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

