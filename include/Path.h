#ifndef PATH_H
#define PATH_H

#include <SFML/Graphics.hpp>
#include <vector>

// Simple waypoint path system for enemies.
// - Waypoints are in screen/world coordinates (same space as Enemy positions)
// - Path moves linearly between waypoints at a constant speed
// - Supports looping or finishing at last waypoint
class Path {
public:
    Path();
    Path(const std::vector<sf::Vector2f>& waypoints, float speed = 100.0f, bool loop = true);

    void setWaypoints(const std::vector<sf::Vector2f>& waypoints);
    void setStart(const sf::Vector2f& startPos);
    void setSpeed(float s);
    void setLoop(bool loop);

    void reset();
    void update(float deltaTime);

    sf::Vector2f getPosition() const;
    bool isFinished() const;

private:
    std::vector<sf::Vector2f> m_waypoints;
    size_t m_targetIndex; // index in waypoints we are moving toward
    sf::Vector2f m_position;
    float m_speed;
    bool m_loop;
    bool m_finished;
};

#endif // PATH_H
