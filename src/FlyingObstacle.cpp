#include "FlyingObstacle.h"

#include <iostream>

#include "TextureCache.h"

FlyingObstacle::FlyingObstacle(const ObstacleSpec& spec) {
    sf::Texture* tex = TextureCache::instance().load(spec.texturePath);
    if (!tex) {
        std::cerr << "FlyingObstacle: missing texture " << spec.texturePath << std::endl;
        return;
    }
    sprite = std::make_unique<sf::Sprite>(*tex);
    sf::Vector2u ts = tex->getSize();
    float tw = static_cast<float>(ts.x);
    float th = static_cast<float>(ts.y);

    sprite->setOrigin(sf::Vector2f(tw / 2.f, th / 2.f));
    sprite->setPosition(sf::Vector2f(spec.x, spec.y));

    if (spec.width > 0.f && spec.height > 0.f) {
        sprite->setScale(sf::Vector2f(spec.width / tw, spec.height / th));
    }

    bounds = sprite->getGlobalBounds();
}

void FlyingObstacle::draw(sf::RenderWindow& window) const {
    if (sprite) {
        window.draw(*sprite);
    }
}

sf::FloatRect FlyingObstacle::getBounds() const {
    if (sprite) {
        return sprite->getGlobalBounds();
    }
    return bounds;
}
