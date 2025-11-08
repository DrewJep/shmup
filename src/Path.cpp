#include "Path.h"
#include <cmath>

Path::Path()
    : m_targetIndex(0), m_position(0.f, 0.f), m_speed(100.f), m_loop(true), m_finished(true) {}

Path::Path(const std::vector<sf::Vector2f>& waypoints, float speed, bool loop)
    : m_waypoints(waypoints), m_targetIndex(0), m_position(0.f, 0.f), m_speed(speed), m_loop(loop), m_finished(false)
{
    if (!m_waypoints.empty()) {
        m_position = m_waypoints.front();
        m_targetIndex = 1 % m_waypoints.size();
    } else {
        m_finished = true;
    }
}

void Path::setWaypoints(const std::vector<sf::Vector2f>& waypoints) {
    m_waypoints = waypoints;
    reset();
}

void Path::setStart(const sf::Vector2f& startPos) {
    if (m_waypoints.empty()) {
        // If no waypoints, start position becomes the sole position
        m_position = startPos;
        m_finished = true;
    } else {
        // Place the internal position at startPos, keep target as first waypoint (or next)
        m_position = startPos;
        // Find nearest waypoint index to start moving toward: choose first waypoint by default
        m_targetIndex = 0;
    }
}

void Path::setSpeed(float s) { m_speed = s; }
void Path::setLoop(bool loop) { m_loop = loop; }

void Path::reset() {
    m_finished = m_waypoints.empty();
    if (!m_waypoints.empty()) {
        m_position = m_waypoints.front();
        m_targetIndex = 1 % m_waypoints.size();
    } else {
        m_targetIndex = 0;
    }
}

void Path::update(float deltaTime) {
    if (m_finished || m_waypoints.empty()) return;

    // Current target
    sf::Vector2f target = m_waypoints[m_targetIndex];

    sf::Vector2f toTarget = target - m_position;
    float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
    if (distSq < 1e-4f) {
        // Arrived at waypoint: advance
        m_position = target;
        // advance index
        m_targetIndex++;
        if (m_targetIndex >= m_waypoints.size()) {
            if (m_loop && !m_waypoints.empty()) {
                m_targetIndex = 0;
            } else {
                m_finished = true;
                return;
            }
        }
        return;
    }

    float dist = std::sqrt(distSq);
    sf::Vector2f dir = toTarget / dist;
    float moveDist = m_speed * deltaTime;

    if (moveDist >= dist) {
        // reach target and advance; keep leftover movement by setting position to target
        m_position = target;
        m_targetIndex++;
        if (m_targetIndex >= m_waypoints.size()) {
            if (m_loop && !m_waypoints.empty()) {
                m_targetIndex = 0;
            } else {
                m_finished = true;
            }
        }
    } else {
        m_position += dir * moveDist;
    }
}

sf::Vector2f Path::getPosition() const { return m_position; }
bool Path::isFinished() const { return m_finished; }
