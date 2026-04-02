#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <string>
#include "Ship.h"
#include "Projectile.h"
#include "Enemy.h"
#include "TextBox.h"
#include "Level.h"
#include "FlyingObstacle.h"

class Game {
public:
    static int windowWidth() { return WINDOW_WIDTH; }
    static int windowHeight() { return WINDOW_HEIGHT; }

    Game();
    ~Game();

    void run();

private:
    enum class GameState { Playing, WaitingDialogueAdvance };

    void processEvents();
    void update(float deltaTime);
    void render();

    void configureWindow();
    void loadPersistentAssets();
    void startBackgroundMusicIfLoaded();

    bool loadLevelFile(const std::string& path);
    void rebuildFlyingObstaclesFromLevel();
    void updateLevelHudLabel();
    void runInitialScriptHead();
    void tryLoadCampaignStartOrDemo();

    void showDialogue(const std::string& text, const std::string& speakerForLog);

    void tickGameplaySystems(float deltaTime);
    void updateBackgroundScroll(float deltaTime);
    void spawnPlayerProjectileIfReady();
    void updateProjectiles(float deltaTime);
    void updateEnemies(float deltaTime);
    void checkCollisions();
    void checkProjectileObstacleCollisions();
    void resolvePlayerFlyingObstacles();
    void applyEnemyShipContactDamage();
    void clampPlayerShipToScreen();
    void updateGameOverIfDead();
    void advanceLevelScriptIfWaveClear();
    void tryLoadNextCampaignLevel();

    struct PlayfieldLayout {
        float playLeft = 0.f;
        float playTop = 0.f;
        float playRight = 0.f;
        float playWidth = 0.f;
        float playHeight = 0.f;
        float sideWidth = 0.f;
    };
    PlayfieldLayout computePlayfieldLayout() const;
    void renderDebugOnce();
    void renderFramePanels(const PlayfieldLayout& layout);
    void renderPlayfieldWorld(const PlayfieldLayout& layout);
    void renderHud(const PlayfieldLayout& layout);

    sf::RenderWindow window;
    static const int PLAY_WIDTH = 320;
    static const int PLAY_HEIGHT = 224;
    static const int WINDOW_WIDTH = PLAY_WIDTH * 2;
    static const int WINDOW_HEIGHT = PLAY_HEIGHT * 2;
    static const std::string WINDOW_TITLE;

    Ship playerShip;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<FlyingObstacle> flyingObstacles;

    Level levelScript;
    size_t campaignLevelIndex = 0;
    bool startedFromCampaign = false;
    std::string levelHudLabel;

    std::unique_ptr<TextBox> activeTextBox;
    GameState gameState = GameState::Playing;

    void drawFloor(sf::RenderWindow& window);
    static const int FLOOR_GRID_SIZE = 20;
    float backgroundScrollX;
    float backgroundScrollY;
    static constexpr float SCROLL_SPEED = 420.0f;

    sf::Clock clock;
    float deltaTime;
    float elapsedTime;

    sf::Font uiFont;
    bool uiHasFont;
    sf::Music backgroundMusic;
    bool musicLoaded;

    bool isRunning;
};

#endif
