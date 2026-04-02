#include "Actor.h"
#include <algorithm>

Actor::Actor(float x, float y, int initialHealth)
    : position(x, y), velocity(0.f, 0.f), health(initialHealth) {}

sf::Vector2f Actor::getPosition() const {
    return position;
}

void Actor::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
}

sf::Vector2f Actor::getVelocity() const {
    return velocity;
}

void Actor::setVelocity(const sf::Vector2f& v) {
    velocity = v;
}

int Actor::getHealth() const {
    return health;
}

void Actor::takeDamage(int amount) {
    health = std::max(0, health - amount);
}

bool Actor::isDead() const {
    return health <= 0;
}
