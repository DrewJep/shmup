#ifndef UFOENEMY_H
#define UFOENEMY_H

#include "Enemy.h"

class UfoEnemy : public Enemy {
public:
    explicit UfoEnemy(float x, float y, float speed = 100.0f);

private:
    void updateVisuals(float deltaTime) override;
    void refreshSpriteRect();

    sf::Texture* sheetTexture = nullptr;
    static constexpr int FRAME_COLS = 2;
    static constexpr int FRAME_ROWS = 3;
    static constexpr int TOTAL_FRAMES = FRAME_COLS * FRAME_ROWS;
    int currentFrame = 0;
    float animationTimer = 0.f;
    float frameDuration = 0.08f;
};

#endif
