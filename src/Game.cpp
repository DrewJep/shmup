#include "Game.h"
#include "IsometricUtils.h"
#include "Projectile.h"
#include "Path.h"
#include "ShootingPattern.h"
#include <iostream>
#include <optional>
#include <cmath>
#include <cstdio>
#include <SFML/Graphics/RenderTexture.hpp>

const std::string Game::WINDOW_TITLE = "Down to Earth: A Shmup With Legs";

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), WINDOW_TITLE),
      playerShip(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 300.0f),
            deltaTime(0.0f),
            elapsedTime(0.0f),
                        isRunning(true),
                        uiHasFont(false),
                        currentLevel(1) {
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    
    // Pre-load projectile texture
    Projectile::loadTexture();

    // Attempt to load UI font (optional) - SFML3 uses openFromFile
        if (uiFont.openFromFile("assets/fonts/Qager-zrlmw.ttf")) {
        uiHasFont = true;
    } else {
        uiHasFont = false;
    }

        // Attempt to load background music from common locations
        musicLoaded = false;
        std::vector<std::string> musicPaths = {
            "assets/sounds/music/test_song.mp3",
            "assets/sounds/test_song.mp3",
            "assets/sound/music/test_song.mp3",
            "assets/sound/test_song.mp3",
            "assets/music/test_song.mp3",
        };

        for (const auto& p : musicPaths) {
            if (backgroundMusic.openFromFile(p)) {
                musicLoaded = true;
                std::cout << "Loaded background music: " << p << std::endl;
                break;
            }
        }

        if (musicLoaded) {
            backgroundMusic.setLooping(true);
            backgroundMusic.play();
        } else {
            std::cout << "Background music not found in expected paths." << std::endl;
        }
    
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
        // Assign shooting patterns: lead enemy shoots radial bursts, followers shoot at player
        if (i == 0) {
            enemies.back()->setShootingPattern(makeRadialPattern(10, 3.0f, 160.0f));
        } else {
            // Faster fire rate for closer trailing enemies
            float rate = 1.2f - i * 0.3f;
            enemies.back()->setShootingPattern(makeDirectAtPlayerPattern(rate, 240.0f, 400.0f, false));
        }
    }

    // Spawn a separate fourth enemy that uses the lingering beam pattern.
    // This enemy is not part of the patrol path and will sit near the top-right area.
    {
        float bx = WINDOW_WIDTH * 0.72f;
        float by = WINDOW_HEIGHT * 0.22f;
        auto beamEnemy = std::make_unique<Enemy>(bx, by, 40.0f);
        // No path set - it will use its simple wandering movement or remain mostly stationary
        // Give it a lingering beam pattern: interval, warningDuration, beamDuration, projSpeed
    // Use a simple direct-at-player pattern instead of the lingering beam
    beamEnemy->setShootingPattern(makeDirectAtPlayerPattern(2.0f, 180.0f, 800.0f, true));
        enemies.push_back(std::move(beamEnemy));
    }
}

// Create a render texture for the retro playfield if desired. We create it lazily
// at first render to avoid early GPU allocation issues on some systems.


Game::~Game() {
    // Stop music if playing. Wrap in try/catch to avoid exceptions escaping destructor
    try {
        if (musicLoaded) {
            backgroundMusic.stop();
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in Game::~Game(): " << ex.what() << std::endl;
        // swallow: do not rethrow from destructor
    } catch (...) {
        std::cerr << "Unknown exception in Game::~Game()" << std::endl;
    }
}

void Game::run() {
    while (isRunning && window.isOpen()) {
        deltaTime = clock.restart().asSeconds();
        elapsedTime += deltaTime;
        
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
            // Forward aim keys (IJKL) to ship for twin-stick ground mode aiming
            playerShip.handleAimInput(keyPressed->code, true);
            
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                window.close();
                isRunning = false;
            }
        }
        
        // Handle key release events
        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            playerShip.handleInput(keyReleased->code, false);
            playerShip.handleAimInput(keyReleased->code, false);
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
    
    // Update enemies (pass player position and allow enemies to spawn projectiles)
    sf::Vector2f playerPos = playerShip.getPosition();
    for (auto it = enemies.begin(); it != enemies.end();) {
        (*it)->update(deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT, playerPos, projectiles);

        // Remove dead enemies
        if ((*it)->isDead()) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
    
    // Check collisions between projectiles and enemies
    checkCollisions();

    // Check collisions between enemies and player ship
    for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
        {
            // Manual AABB overlap check (SFML 3 removed FloatRect::intersects helper in some configs)
            sf::FloatRect a = (*enemyIt)->getBounds();
            sf::FloatRect b = playerShip.getBounds();
            bool xOverlap = (a.position.x < b.position.x + b.size.x) && (b.position.x < a.position.x + a.size.x);
            bool yOverlap = (a.position.y < b.position.y + b.size.y) && (b.position.y < a.position.y + a.size.y);
            if (xOverlap && yOverlap) {
                // Damage player and enemy (simple rules: both take 1)
                playerShip.takeDamage(1);
                (*enemyIt)->takeDamage(1);
            }
        }
        ++enemyIt;
    }
    
    // Keep ship within screen bounds
    sf::Vector2f pos = playerShip.getPosition();
    float shipRadius = 15.0f;
    
    if (pos.x < shipRadius) playerShip.setPosition(shipRadius, pos.y);
    if (pos.x > WINDOW_WIDTH - shipRadius) playerShip.setPosition(WINDOW_WIDTH - shipRadius, pos.y);
    if (pos.y < shipRadius) playerShip.setPosition(pos.x, shipRadius);
    if (pos.y > WINDOW_HEIGHT - shipRadius) playerShip.setPosition(pos.x, WINDOW_HEIGHT - shipRadius);

    // End game if player health is 0
    if (playerShip.getHealth() <= 0) {
        isRunning = false;
        window.close();
    }
}

void Game::render() {
    // Clear with a dark background (space-like)
    window.clear(sf::Color(20, 20, 40));

    // One-time diagnostic print to help debug drawing issues
    static bool debugPrinted = false;
    if (!debugPrinted) {
        debugPrinted = true;
        std::cout << "Render diagnostic: projectiles=" << projectiles.size()
                  << " enemies=" << enemies.size()
                  << " playerPos=(" << playerShip.getPosition().x << "," << playerShip.getPosition().y << ")"
                  << " musicLoaded=" << musicLoaded << std::endl;
    }

    // Draw play area borders and UI panels
    // Compute an integer scale for the retro play area so it scales crisply.
    int scale = std::min(WINDOW_WIDTH / PLAY_WIDTH, WINDOW_HEIGHT / PLAY_HEIGHT);
    if (scale < 1) scale = 1;
    float playWidth = static_cast<float>(PLAY_WIDTH * scale);
    float playHeight = static_cast<float>(PLAY_HEIGHT * scale);
    float playLeft = (WINDOW_WIDTH - playWidth) / 2.0f;
    // Center vertically as well so the playfield feels like a centered arcade viewport
    float playTop = (WINDOW_HEIGHT - playHeight) / 2.0f;
    float playRight = playLeft + playWidth;

    // Side panels thickness
    float sideWidth = playLeft; // left and right panel width

    // Draw background for panels
    sf::RectangleShape leftPanel(sf::Vector2f(sideWidth, static_cast<float>(WINDOW_HEIGHT)));
    leftPanel.setPosition(sf::Vector2f(0.f, 0.f));
    leftPanel.setFillColor(sf::Color(30, 30, 45));
    window.draw(leftPanel);

    sf::RectangleShape rightPanel(sf::Vector2f(sideWidth, static_cast<float>(WINDOW_HEIGHT)));
    rightPanel.setPosition(sf::Vector2f(playRight, 0.f));
    rightPanel.setFillColor(sf::Color(30, 30, 45));
    window.draw(rightPanel);

    // Draw play area bg (slightly different color)
    sf::RectangleShape playArea(sf::Vector2f(playWidth, playHeight));
    playArea.setPosition(sf::Vector2f(playLeft, playTop));
    playArea.setFillColor(sf::Color(124, 252, 0));
    window.draw(playArea);

    // Draw thin top bar for HUD (time, level)
    float topBarH = 28.0f;
    sf::RectangleShape topBar(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), topBarH));
    topBar.setPosition(sf::Vector2f(0.f, 0.f));
    topBar.setFillColor(sf::Color(25, 25, 40));
    topBar.setOutlineColor(sf::Color(80, 80, 90));
    topBar.setOutlineThickness(1.0f);
    window.draw(topBar);

    // Save current view and set a view clipped to the play area so drawFloor uses the same screen coords
    sf::View prevView = window.getView();
    sf::View playView = prevView;
    // Keep the same size as the window view but translate so (0,0) for drawing maps to the play area
    playView.setViewport(sf::FloatRect(
        sf::Vector2f(playLeft / static_cast<float>(WINDOW_WIDTH), playTop / static_cast<float>(WINDOW_HEIGHT)),
        sf::Vector2f(playWidth / static_cast<float>(WINDOW_WIDTH), playHeight / static_cast<float>(WINDOW_HEIGHT))
    ));
    window.setView(playView);

    // Draw floor inside play area
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

    // Restore previous view to draw UI elements in screen coordinates
    window.setView(prevView);

    // Draw UI: health bar (vertical stacked) in left panel
    float uiMargin = 16.0f;
    float healthPanelX = uiMargin;
    float healthPanelY = uiMargin;
    float healthPanelW = sideWidth - uiMargin * 2.0f;
    float healthPanelH = 120.0f;

    // Panel background
    sf::RectangleShape hpBg(sf::Vector2f(healthPanelW, healthPanelH));
    hpBg.setPosition(sf::Vector2f(healthPanelX, healthPanelY));
    hpBg.setFillColor(sf::Color(12, 12, 20));
    hpBg.setOutlineColor(sf::Color(80, 80, 90));
    hpBg.setOutlineThickness(2.0f);
    window.draw(hpBg);

    // Draw stacked HP segments (top to bottom)
    int maxHP = 20;
    int hp = playerShip.getHealth();
    float segmentH = (healthPanelH - 8.0f) / static_cast<float>(maxHP);
    for (int i = 0; i < maxHP; ++i) {
        float segX = healthPanelX + 4.0f;
        float segY = healthPanelY + 4.0f + i * segmentH;
        float segW = healthPanelW - 8.0f;
        float segH = segmentH - 4.0f;

        sf::RectangleShape seg(sf::Vector2f(segW, segH));
        seg.setPosition(sf::Vector2f(segX, segY));
        if (i < hp) {
            // Filled segment (from top down)
            seg.setFillColor(sf::Color(200, 30, 30));
        } else {
            seg.setFillColor(sf::Color(60, 60, 70));
        }
        seg.setOutlineColor(sf::Color(30, 30, 40));
        seg.setOutlineThickness(1.0f);
        window.draw(seg);
    }

    // Draw ship mode below the HP panel
    if (uiHasFont) {
        std::string modeStr = (playerShip.getMode() == Ship::Mode::Air) ? "MODE: AIR" : "MODE: GROUND";
        sf::Text modeText(uiFont, modeStr, 14);
        modeText.setFillColor(sf::Color::White);
        modeText.setPosition(sf::Vector2f(healthPanelX, healthPanelY + healthPanelH + 8.0f));
        window.draw(modeText);
    }

    // Right panel: show weapon slots (primary, special, defense) and top bar shows time/level
    if (uiHasFont) {
        // Weapons stacked vertically on the right panel
        float weaponX = playRight + uiMargin;
        float weaponY = uiMargin;
        float iconW = healthPanelW;
        float iconH = 28.0f;

        auto drawWeapon = [&](const std::string &name, const sf::Color &col, float yOff) {
            sf::RectangleShape icon(sf::Vector2f(iconW, iconH));
            icon.setPosition(sf::Vector2f(weaponX, weaponY + yOff));
            icon.setFillColor(col);
            icon.setOutlineColor(sf::Color(30, 30, 40));
            icon.setOutlineThickness(1.0f);
            window.draw(icon);

            sf::Text t(uiFont, name, 14);
            t.setFillColor(sf::Color::White);
            t.setPosition(sf::Vector2f(weaponX + 6.0f, weaponY + yOff + 6.0f));
            window.draw(t);
        };

        drawWeapon("Primary", sf::Color(160,160,200), 0.0f);
        drawWeapon("Special", sf::Color(200,160,160), iconH + 6.0f);
        drawWeapon("Defense", sf::Color(160,200,160), 2*(iconH + 6.0f));

        // Draw time and level on the top bar
        char buf[64];
        int seconds = static_cast<int>(elapsedTime);
        std::snprintf(buf, sizeof(buf), "%02d:%02d", seconds / 60, seconds % 60);
        sf::Text timeText(uiFont, buf, 14);
        timeText.setFillColor(sf::Color::White);
        timeText.setPosition(sf::Vector2f(playLeft + 8.0f, 4.0f));
        window.draw(timeText);

        char buf2[32];
        std::snprintf(buf2, sizeof(buf2), "Level %d", currentLevel);
        sf::Text levelText(uiFont, buf2, 14);
        levelText.setFillColor(sf::Color::White);
        // Right-align level text on top bar
        levelText.setPosition(sf::Vector2f(playRight - 80.0f, 4.0f));
        window.draw(levelText);
    }

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
        // Only player-owned projectiles should damage enemies
        if ((*projIt)->getOwner() == Projectile::Owner::Player) {
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
        }
        
        // Remove projectile if it hit an enemy
        if (projectileHit) {
            projIt = projectiles.erase(projIt);
        } else {
            ++projIt;
        }
    }

    // Check enemy projectiles against the player
    for (auto projIt = projectiles.begin(); projIt != projectiles.end();) {
        if ((*projIt)->getOwner() == Projectile::Owner::Enemy) {
            if ((*projIt)->checkCollision(playerShip.getBounds())) {
                // Damage player and remove projectile
                playerShip.takeDamage(1);
                projIt = projectiles.erase(projIt);
                continue;
            }
        }
        ++projIt;
    }
}

