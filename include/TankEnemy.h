#ifndef TANKENEMY_H
#define TANKENEMY_H

#include "Enemy.h"

class TankEnemy : public Enemy {
public:
    explicit TankEnemy(float x, float y, float speed = 100.0f);

private:
    void updateVisuals(float deltaTime) override;
    void refreshTankSpriteRect();
    void refreshUfoFallbackSpriteRect();

    sf::Texture* sheetTexture = nullptr;
    bool m_ufoFallback = false;

    static constexpr int FRAME_COLS = 2;
    static constexpr int FRAME_ROWS = 3;
    static constexpr int TOTAL_FRAMES = FRAME_COLS * FRAME_ROWS;
    int currentFrame = 0;
    float animationTimer = 0.f;
    float frameDuration = 0.08f;
};

#endif
