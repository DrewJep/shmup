#include "Game.h"
#include "Collision.h"
#include "GameResources.h"
#include "IsometricUtils.h"
#include "LevelSpawner.h"
#include "Projectile.h"
#include <iostream>
#include <optional>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
const char* kCampaignLevels[] = {
    "assets/levelScript/1-1.json",
    "assets/levelScript/1-2.json",
};
constexpr size_t kCampaignLevelCount = sizeof(kCampaignLevels) / sizeof(kCampaignLevels[0]);
} // namespace

const std::string Game::WINDOW_TITLE = "Down to Earth: A Shmup With Legs";

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), WINDOW_TITLE),
      playerShip(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 300.0f),
      deltaTime(0.0f),
      elapsedTime(0.0f),
      backgroundScrollX(0.0f),
      backgroundScrollY(0.0f),
      isRunning(true),
      uiHasFont(false) {
    configureWindow();
    loadPersistentAssets();
    tryLoadCampaignStartOrDemo();
    runInitialScriptHead();
}

void Game::configureWindow() {
    GameResources::configureWindow(window);
}

void Game::loadPersistentAssets() {
    GameResources::preloadCoreTextures();
    Projectile::loadTexture();

    uiHasFont = GameResources::tryLoadUiFont(uiFont);

    musicLoaded = GameResources::tryOpenBackgroundMusic(backgroundMusic);
    startBackgroundMusicIfLoaded();
}

void Game::startBackgroundMusicIfLoaded() {
    if (musicLoaded) {
        backgroundMusic.setLooping(true);
        backgroundMusic.play();
    }
}

bool Game::loadLevelFile(const std::string& path) {
    if (!levelScript.loadFromFile(path)) {
        return false;
    }
    rebuildFlyingObstaclesFromLevel();
    updateLevelHudLabel();
    return true;
}

void Game::rebuildFlyingObstaclesFromLevel() {
    flyingObstacles.clear();
    for (const auto& spec : levelScript.obstacleSpecs()) {
        flyingObstacles.emplace_back(spec);
    }
}

void Game::updateLevelHudLabel() {
    if (!levelScript.levelId().empty()) {
        levelHudLabel = levelScript.levelId();
    } else {
        levelHudLabel = std::to_string(static_cast<int>(campaignLevelIndex) + 1);
    }
}

void Game::tryLoadCampaignStartOrDemo() {
    campaignLevelIndex = 0;
    startedFromCampaign = false;
    if (loadLevelFile(kCampaignLevels[0])) {
        startedFromCampaign = true;
        std::cout << "Loaded campaign: " << kCampaignLevels[0] << std::endl;
        return;
    }
    if (loadLevelFile("assets/levelScript/demo.json")) {
        std::cout << "Loaded fallback demo level (demo.json)\n";
        return;
    }
    std::cerr << "Could not load 1-1.json or demo.json\n";
}

void Game::runInitialScriptHead() {
    if (levelScript.hasNext()) {
        const LevelEvent* peekEv = levelScript.peek();
        if (peekEv && peekEv->kind == LevelEvent::Kind::Spawn) {
            auto evOpt = levelScript.next();
            if (evOpt) {
                for (const auto& spec : evOpt->enemies) {
                    spawnFromSpec(enemies, spec, WINDOW_WIDTH, WINDOW_HEIGHT);
                }
            }
        }
    }

    if (levelScript.hasNext()) {
        auto evOpt = levelScript.next();
        if (evOpt && evOpt->kind == LevelEvent::Kind::Dialogue) {
            showDialogue(evOpt->text, evOpt->speaker);
        }
    }
}

void Game::showDialogue(const std::string& text, const std::string& speakerForLog) {
    if (uiHasFont) {
        float tbW = static_cast<float>(WINDOW_WIDTH) - 40.0f;
        float tbH = 96.0f;
        float tbX = 20.0f;
        float tbY = static_cast<float>(WINDOW_HEIGHT) - tbH - 20.0f;
        activeTextBox = std::make_unique<TextBox>(tbX, tbY, tbW, tbH, uiFont, 16);
        activeTextBox->setText(text);
        gameState = GameState::WaitingDialogueAdvance;
        std::cout << "Dialogue (" << speakerForLog << "): " << text << std::endl;
    } else {
        std::cout << "Dialogue: " << speakerForLog << ": " << text << std::endl;
    }
}


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
            if (keyPressed->code == sf::Keyboard::Key::Space &&
                gameState == GameState::WaitingDialogueAdvance) {
                activeTextBox.reset();
                gameState = GameState::Playing;
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
    tickGameplaySystems(deltaTime);
}

void Game::tickGameplaySystems(float deltaTime) {
    playerShip.updateInput();
    updateBackgroundScroll(deltaTime);
    spawnPlayerProjectileIfReady();
    playerShip.updateMouseAim(window);
    playerShip.update(deltaTime);
    updateProjectiles(deltaTime);
    updateEnemies(deltaTime);
    checkProjectileObstacleCollisions();
    checkCollisions();
    resolvePlayerFlyingObstacles();
    applyEnemyShipContactDamage();
    clampPlayerShipToScreen();
    updateGameOverIfDead();
    advanceLevelScriptIfWaveClear();
}

void Game::updateBackgroundScroll(float deltaTime) {
    if (playerShip.getMode() != Ship::Mode::Air) return;

    backgroundScrollY -= SCROLL_SPEED * 0.5f * deltaTime;
    while (backgroundScrollY >= IsometricUtils::TILE_HEIGHT) backgroundScrollY -= IsometricUtils::TILE_HEIGHT;
    while (backgroundScrollY < 0.0f) backgroundScrollY += IsometricUtils::TILE_HEIGHT;

    backgroundScrollX -= SCROLL_SPEED * deltaTime;
    while (backgroundScrollX >= IsometricUtils::TILE_WIDTH) backgroundScrollX -= IsometricUtils::TILE_WIDTH;
    while (backgroundScrollX < 0.0f) backgroundScrollX += IsometricUtils::TILE_WIDTH;
}

void Game::spawnPlayerProjectileIfReady() {
    if (!playerShip.shouldShoot()) return;

    sf::Vector2f shipPos = playerShip.getPosition();
    float angle = playerShip.getForwardAngle();
    constexpr float offsetDistance = 30.0f;
    float spawnX = shipPos.x + std::cos(angle) * offsetDistance;
    float spawnY = shipPos.y + std::sin(angle) * offsetDistance;
    projectiles.push_back(std::make_unique<Projectile>(spawnX, spawnY, angle));
}

void Game::updateProjectiles(float deltaTime) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        (*it)->update(deltaTime);
        if ((*it)->isOffScreen(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::updateEnemies(float deltaTime) {
    sf::Vector2f playerPos = playerShip.getPosition();
    for (auto it = enemies.begin(); it != enemies.end();) {
        (*it)->update(deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT, playerPos, projectiles);
        if ((*it)->isDead()) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::applyEnemyShipContactDamage() {
    for (auto& enemy : enemies) {
        if (Collision::rectsOverlap(enemy->getBounds(), playerShip.getBounds())) {
            playerShip.takeDamage(1);
            enemy->takeDamage(1);
        }
    }
}

void Game::clampPlayerShipToScreen() {
    sf::Vector2f pos = playerShip.getPosition();
    constexpr float shipRadius = 15.0f;

    if (pos.x < shipRadius) playerShip.setPosition(shipRadius, pos.y);
    if (pos.x > WINDOW_WIDTH - shipRadius) playerShip.setPosition(WINDOW_WIDTH - shipRadius, pos.y);
    if (pos.y < shipRadius) playerShip.setPosition(pos.x, shipRadius);
    if (pos.y > WINDOW_HEIGHT - shipRadius) playerShip.setPosition(pos.x, WINDOW_HEIGHT - shipRadius);
}

void Game::updateGameOverIfDead() {
    if (playerShip.getHealth() <= 0) {
        isRunning = false;
        window.close();
    }
}

void Game::checkProjectileObstacleCollisions() {
    if (flyingObstacles.empty()) return;

    for (auto it = projectiles.begin(); it != projectiles.end();) {
        bool hit = false;
        sf::FloatRect pb = (*it)->getBounds();
        for (const auto& o : flyingObstacles) {
            if (Collision::rectsOverlap(pb, o.getBounds())) {
                hit = true;
                break;
            }
        }
        if (hit) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::resolvePlayerFlyingObstacles() {
    if (playerShip.getMode() != Ship::Mode::Air || flyingObstacles.empty()) return;

    for (int iter = 0; iter < 4; ++iter) {
        sf::FloatRect sb = playerShip.getBounds();
        bool moved = false;
        for (const auto& o : flyingObstacles) {
            sf::FloatRect ob = o.getBounds();
            if (!Collision::rectsOverlap(sb, ob)) continue;

            float dxLeft = (ob.position.x + ob.size.x) - sb.position.x;
            float dxRight = (sb.position.x + sb.size.x) - ob.position.x;
            float dyUp = (ob.position.y + ob.size.y) - sb.position.y;
            float dyDown = (sb.position.y + sb.size.y) - ob.position.y;
            float vals[4] = {dxLeft, dxRight, dyUp, dyDown};
            int minIdx = 0;
            for (int i = 1; i < 4; ++i) {
                if (vals[i] < vals[minIdx]) minIdx = i;
            }

            sf::Vector2f p = playerShip.getPosition();
            if (minIdx == 0) {
                p.x += dxLeft;
            } else if (minIdx == 1) {
                p.x -= dxRight;
            } else if (minIdx == 2) {
                p.y += dyUp;
            } else {
                p.y -= dyDown;
            }
            playerShip.setPosition(p.x, p.y);
            moved = true;
            break;
        }
        if (!moved) break;
    }
}

void Game::advanceLevelScriptIfWaveClear() {
    if (!enemies.empty() || gameState != GameState::Playing) return;

    if (levelScript.hasNext()) {
        auto evOpt = levelScript.next();
        if (!evOpt) return;

        LevelEvent ev = *evOpt;
        if (ev.kind == LevelEvent::Kind::Dialogue) {
            showDialogue(ev.text, ev.speaker);
        } else if (ev.kind == LevelEvent::Kind::Spawn) {
            for (const auto& spec : ev.enemies) {
                spawnFromSpec(enemies, spec, WINDOW_WIDTH, WINDOW_HEIGHT);
            }
        }
        return;
    }

    if (levelScript.finishedAllEvents()) {
        tryLoadNextCampaignLevel();
    }
}

void Game::tryLoadNextCampaignLevel() {
    if (!startedFromCampaign) return;
    if (campaignLevelIndex + 1 >= kCampaignLevelCount) {
        std::cout << "Campaign complete (no further levels).\n";
        return;
    }

    campaignLevelIndex++;
    projectiles.clear();
    enemies.clear();

    if (!loadLevelFile(kCampaignLevels[campaignLevelIndex])) {
        std::cerr << "Failed to load " << kCampaignLevels[campaignLevelIndex] << std::endl;
        campaignLevelIndex--;
        return;
    }

    std::cout << "Started level file: " << kCampaignLevels[campaignLevelIndex] << std::endl;
    runInitialScriptHead();
}

void Game::render() {
    window.clear(sf::Color(20, 20, 40));
    renderDebugOnce();

    const PlayfieldLayout layout = computePlayfieldLayout();
    renderFramePanels(layout);
    renderPlayfieldWorld(layout);
    renderHud(layout);

    if (activeTextBox) {
        activeTextBox->draw(window);
    }

    window.display();
}

Game::PlayfieldLayout Game::computePlayfieldLayout() const {
    PlayfieldLayout L;
    int scale = std::min(WINDOW_WIDTH / PLAY_WIDTH, WINDOW_HEIGHT / PLAY_HEIGHT);
    if (scale < 1) scale = 1;
    L.playWidth = static_cast<float>(PLAY_WIDTH * scale);
    L.playHeight = static_cast<float>(PLAY_HEIGHT * scale);
    L.playLeft = (WINDOW_WIDTH - L.playWidth) / 2.0f;
    L.playTop = (WINDOW_HEIGHT - L.playHeight) / 2.0f;
    L.playRight = L.playLeft + L.playWidth;
    L.sideWidth = L.playLeft;
    return L;
}

void Game::renderDebugOnce() {
    static bool debugPrinted = false;
    if (debugPrinted) return;
    debugPrinted = true;
    std::cout << "Render diagnostic: projectiles=" << projectiles.size()
              << " enemies=" << enemies.size()
              << " playerPos=(" << playerShip.getPosition().x << "," << playerShip.getPosition().y << ")"
              << " musicLoaded=" << musicLoaded << std::endl;
}

void Game::renderFramePanels(const PlayfieldLayout& layout) {
    sf::RectangleShape leftPanel(sf::Vector2f(layout.sideWidth, static_cast<float>(WINDOW_HEIGHT)));
    leftPanel.setPosition(sf::Vector2f(0.f, 0.f));
    leftPanel.setFillColor(sf::Color(30, 30, 45));
    window.draw(leftPanel);

    sf::RectangleShape rightPanel(sf::Vector2f(layout.sideWidth, static_cast<float>(WINDOW_HEIGHT)));
    rightPanel.setPosition(sf::Vector2f(layout.playRight, 0.f));
    rightPanel.setFillColor(sf::Color(30, 30, 45));
    window.draw(rightPanel);

    sf::RectangleShape playArea(sf::Vector2f(layout.playWidth, layout.playHeight));
    playArea.setPosition(sf::Vector2f(layout.playLeft, layout.playTop));
    playArea.setFillColor(sf::Color(17, 154, 58));
    window.draw(playArea);

    constexpr float topBarH = 28.0f;
    sf::RectangleShape topBar(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), topBarH));
    topBar.setPosition(sf::Vector2f(0.f, 0.f));
    topBar.setFillColor(sf::Color(25, 25, 40));
    topBar.setOutlineColor(sf::Color(80, 80, 90));
    topBar.setOutlineThickness(1.0f);
    window.draw(topBar);
}

void Game::renderPlayfieldWorld(const PlayfieldLayout& layout) {
    sf::View prevView = window.getView();
    sf::View playView = prevView;
    playView.setViewport(sf::FloatRect(
        sf::Vector2f(layout.playLeft / static_cast<float>(WINDOW_WIDTH),
                     layout.playTop / static_cast<float>(WINDOW_HEIGHT)),
        sf::Vector2f(layout.playWidth / static_cast<float>(WINDOW_WIDTH),
                       layout.playHeight / static_cast<float>(WINDOW_HEIGHT))));
    window.setView(playView);

    drawFloor(window);

    for (const auto& obs : flyingObstacles) {
        obs.draw(window);
    }

    for (const auto& projectile : projectiles) {
        projectile->draw(window);
    }
    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }
    playerShip.draw(window);

    window.setView(prevView);
}

void Game::renderHud(const PlayfieldLayout& layout) {
    constexpr float uiMargin = 16.0f;
    float healthPanelX = uiMargin;
    float healthPanelY = uiMargin;
    float healthPanelW = layout.sideWidth - uiMargin * 2.0f;
    float healthPanelH = 120.0f;

    sf::RectangleShape hpBg(sf::Vector2f(healthPanelW, healthPanelH));
    hpBg.setPosition(sf::Vector2f(healthPanelX, healthPanelY));
    hpBg.setFillColor(sf::Color(12, 12, 20));
    hpBg.setOutlineColor(sf::Color(80, 80, 90));
    hpBg.setOutlineThickness(2.0f);
    window.draw(hpBg);

    constexpr int maxHP = 20;
    int hp = playerShip.getHealth();
    float segmentH = (healthPanelH - 8.0f) / static_cast<float>(maxHP);
    for (int i = 0; i < maxHP; ++i) {
        float segX = healthPanelX + 4.0f;
        float segY = healthPanelY + 4.0f + i * segmentH;
        float segW = healthPanelW - 8.0f;
        float segH = segmentH - 4.0f;

        sf::RectangleShape seg(sf::Vector2f(segW, segH));
        seg.setPosition(sf::Vector2f(segX, segY));
        seg.setFillColor(i < hp ? sf::Color(200, 30, 30) : sf::Color(60, 60, 70));
        seg.setOutlineColor(sf::Color(30, 30, 40));
        seg.setOutlineThickness(1.0f);
        window.draw(seg);
    }

    if (!uiHasFont) return;

    std::string modeStr = (playerShip.getMode() == Ship::Mode::Air) ? "MODE: AIR" : "MODE: GROUND";
    sf::Text modeText(uiFont, modeStr, 14);
    modeText.setFillColor(sf::Color::White);
    modeText.setPosition(sf::Vector2f(healthPanelX, healthPanelY + healthPanelH + 8.0f));
    window.draw(modeText);

    float weaponX = layout.playRight + uiMargin;
    float weaponY = uiMargin;
    float iconW = healthPanelW;
    float iconH = 28.0f;

    auto drawWeapon = [&](const std::string& name, const sf::Color& col, float yOff) {
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

    drawWeapon("Primary", sf::Color(160, 160, 200), 0.0f);
    drawWeapon("Special", sf::Color(200, 160, 160), iconH + 6.0f);
    drawWeapon("Defense", sf::Color(160, 200, 160), 2.0f * (iconH + 6.0f));

    char buf[64];
    int seconds = static_cast<int>(elapsedTime);
    std::snprintf(buf, sizeof(buf), "%02d:%02d", seconds / 60, seconds % 60);
    sf::Text timeText(uiFont, buf, 14);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(sf::Vector2f(layout.playLeft + 8.0f, 4.0f));
    window.draw(timeText);

    sf::Text levelText(uiFont, levelHudLabel.empty() ? "—" : levelHudLabel, 14);
    levelText.setFillColor(sf::Color::White);
    levelText.setPosition(sf::Vector2f(layout.playRight - 80.0f, 4.0f));
    window.draw(levelText);
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

    // Offset to center the grid and apply scroll offsets
    float offsetX = WINDOW_WIDTH / 2.0f + backgroundScrollX;
    float offsetY = WINDOW_HEIGHT / 3.0f - backgroundScrollY; // Position floor in lower portion of screen

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
            // Offset coordinates by scroll position before converting to screen space
            float scrolledX = worldX;
            float scrolledY = worldY;
            
            // Convert world coordinates to screen space with scroll offsets
            sf::Vector2f center = IsometricUtils::worldToScreen(scrolledX, scrolledY);
            center.x += offsetX;
            center.y += offsetY;
            
            // Top corner (half tile up)
            sf::Vector2f top = IsometricUtils::worldToScreen(scrolledX, scrolledY - 0.5f);
            top.x += offsetX;
            top.y += offsetY;
            
            // Right corner (half tile right)
            sf::Vector2f right = IsometricUtils::worldToScreen(scrolledX + 0.5f, scrolledY);
            right.x += offsetX;
            right.y += offsetY;
            
            // Bottom corner (half tile down)
            sf::Vector2f bottom = IsometricUtils::worldToScreen(scrolledX, scrolledY + 0.5f);
            bottom.x += offsetX;
            bottom.y += offsetY;
            
            // Left corner (half tile left)
            sf::Vector2f left = IsometricUtils::worldToScreen(scrolledX - 0.5f, scrolledY);
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

