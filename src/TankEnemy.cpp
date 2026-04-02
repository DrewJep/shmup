#include "TankEnemy.h"

#include "TextureCache.h"

TankEnemy::TankEnemy(float x, float y, float speed) : Enemy(x, y, speed) {
    auto& cache = TextureCache::instance();
    sheetTexture = cache.load("assets/characters/tank.png");
    if (sheetTexture) {
        sprite = std::make_unique<sf::Sprite>(*sheetTexture);
        refreshTankSpriteRect();
        sprite->setOrigin(
            sf::Vector2f(sheetTexture->getSize().x / 2.f, sheetTexture->getSize().y / 2.f));
        sprite->setPosition(position);
        return;
    }

    sheetTexture = cache.load("assets/characters/ufo.png");
    m_ufoFallback = true;
    if (sheetTexture) {
        sprite = std::make_unique<sf::Sprite>(*sheetTexture);
        refreshUfoFallbackSpriteRect();
        sf::Vector2u texSize = sheetTexture->getSize();
        int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
        int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;
        sprite->setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));
        sprite->setPosition(position);
    }
}

void TankEnemy::refreshTankSpriteRect() {
    if (!sheetTexture || !sprite) return;
    sf::Vector2u texSize = sheetTexture->getSize();
    sprite->setTextureRect(
        sf::IntRect({0, 0}, sf::Vector2i(static_cast<int>(texSize.x), static_cast<int>(texSize.y))));
}

void TankEnemy::refreshUfoFallbackSpriteRect() {
    if (!sheetTexture || !sprite) return;

    sf::Vector2u texSize = sheetTexture->getSize();
    int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
    int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;

    int col = currentFrame % FRAME_COLS;
    int row = currentFrame / FRAME_COLS;

    sprite->setTextureRect(sf::IntRect({col * frameWidth, row * frameHeight}, {frameWidth, frameHeight}));
}

void TankEnemy::updateVisuals(float deltaTime) {
    if (!m_ufoFallback) return;

    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer = 0.0f;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        refreshUfoFallbackSpriteRect();
    }
}
