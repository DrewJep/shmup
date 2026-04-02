#ifndef ACTOR_H
#define ACTOR_H

#include <SFML/Graphics.hpp>

// Shared transform and health for player ship and enemies.
class Actor {
public:
    Actor(float x, float y, int initialHealth);
    virtual ~Actor() = default;

    sf::Vector2f getPosition() const;
    virtual void setPosition(float x, float y);

    sf::Vector2f getVelocity() const;
    void setVelocity(const sf::Vector2f& v);

    int getHealth() const;
    void takeDamage(int amount);
    bool isDead() const;

    virtual sf::FloatRect getBounds() const = 0;

protected:
    sf::Vector2f position;
    sf::Vector2f velocity;
    int health;
};

#endif
