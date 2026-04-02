#ifndef COLLISION_H
#define COLLISION_H

#include <SFML/Graphics.hpp>

namespace Collision {

bool rectsOverlap(const sf::FloatRect& a, const sf::FloatRect& b);

}

#endif
