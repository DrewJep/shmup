#include "Collision.h"

namespace Collision {

bool rectsOverlap(const sf::FloatRect& a, const sf::FloatRect& b) {
    bool xOverlap = (a.position.x < b.position.x + b.size.x) && (b.position.x < a.position.x + a.size.x);
    bool yOverlap = (a.position.y < b.position.y + b.size.y) && (b.position.y < a.position.y + a.size.y);
    return xOverlap && yOverlap;
}

} // namespace Collision
