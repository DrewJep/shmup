#ifndef FLYINGOBSTACLE_H
#define FLYINGOBSTACLE_H

#include <SFML/Graphics.hpp>
#include <memory>

#include "Level.h"

// Solid scenery in air mode: blocks player movement and all projectiles.
class FlyingObstacle {
public:
    explicit FlyingObstacle(const ObstacleSpec& spec);

    void draw(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;

private:
    std::unique_ptr<sf::Sprite> sprite;
    sf::FloatRect bounds;
};

#endif
