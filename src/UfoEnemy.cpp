#include "UfoEnemy.h"

#include "TextureCache.h"

UfoEnemy::UfoEnemy(float x, float y, float speed) : Enemy(x, y, speed) {
    sheetTexture = TextureCache::instance().load("assets/characters/ufo.png");
    if (sheetTexture) {
        sprite = std::make_unique<sf::Sprite>(*sheetTexture);
        refreshSpriteRect();
        sf::Vector2u texSize = sheetTexture->getSize();
        int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
        int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;
        sprite->setOrigin(sf::Vector2f(frameWidth / 2.f, frameHeight / 2.f));
        sprite->setPosition(position);
    }
}

void UfoEnemy::refreshSpriteRect() {
    if (!sheetTexture || !sprite) return;

    sf::Vector2u texSize = sheetTexture->getSize();
    int frameWidth = static_cast<int>(texSize.x) / FRAME_COLS;
    int frameHeight = static_cast<int>(texSize.y) / FRAME_ROWS;

    int col = currentFrame % FRAME_COLS;
    int row = currentFrame / FRAME_COLS;

    sprite->setTextureRect(sf::IntRect({col * frameWidth, row * frameHeight}, {frameWidth, frameHeight}));
}

void UfoEnemy::updateVisuals(float deltaTime) {
    animationTimer += deltaTime;

    if (animationTimer >= frameDuration) {
        animationTimer = 0.0f;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        refreshSpriteRect();
    }
}
