#include "ShootingPattern.h"
#include "Projectile.h"
#include <cmath>
#include <iostream>

// Direct shot at player every fireRate seconds. Optionally only when player within activeRadius.
class DirectAtPlayerPattern : public ShootingPattern {
public:
    DirectAtPlayerPattern(float fireRate = 1.0f, float projSpeed = 220.0f, float activeRadius = 400.0f, bool always = false)
    : m_fireRate(fireRate), m_timer(0.0f), m_projSpeed(projSpeed), m_activeRadius(activeRadius), m_always(always) {}

    void update(float deltaTime, const sf::Vector2f& enemyPos, const sf::Vector2f& playerPos,
                std::vector<std::unique_ptr<Projectile>>& projectiles) override {
        m_timer += deltaTime;
        float dx = playerPos.x - enemyPos.x;
        float dy = playerPos.y - enemyPos.y;
        float dist2 = dx*dx + dy*dy;
        if (!m_always && dist2 > m_activeRadius * m_activeRadius) return;

        if (m_timer >= m_fireRate) {
            m_timer = 0.0f;
            float angle = std::atan2(dy, dx);
            projectiles.push_back(std::make_unique<Projectile>(enemyPos.x, enemyPos.y, angle, m_projSpeed, Projectile::Owner::Enemy));
        }
    }

private:
    float m_fireRate;
    float m_timer;
    float m_projSpeed;
    float m_activeRadius;
    bool m_always;
};

// Radial burst pattern: fire N projectiles evenly around every interval
class RadialPattern : public ShootingPattern {
public:
    RadialPattern(int count = 8, float interval = 2.0f, float projSpeed = 160.0f)
    : m_count(count), m_interval(interval), m_timer(0.0f), m_projSpeed(projSpeed) {}

    void update(float deltaTime, const sf::Vector2f& enemyPos, const sf::Vector2f& /*playerPos*/,
                std::vector<std::unique_ptr<Projectile>>& projectiles) override {
        m_timer += deltaTime;
        if (m_timer >= m_interval) {
            m_timer = 0.0f;
            for (int i = 0; i < m_count; ++i) {
                float angle = (2.0f * 3.14159265f * i) / static_cast<float>(m_count);
                projectiles.push_back(std::make_unique<Projectile>(enemyPos.x, enemyPos.y, angle, m_projSpeed, Projectile::Owner::Enemy));
            }
        }
    }

private:
    int m_count;
    float m_interval;
    float m_timer;
    float m_projSpeed;
};

// Factory helpers
std::unique_ptr<ShootingPattern> makeDirectAtPlayerPattern(float fireRate, float projSpeed, float activeRadius, bool always) {
    return std::make_unique<DirectAtPlayerPattern>(fireRate, projSpeed, activeRadius, always);
}

std::unique_ptr<ShootingPattern> makeRadialPattern(int count, float interval, float projSpeed) {
    return std::make_unique<RadialPattern>(count, interval, projSpeed);
}

// Lingering beam pattern: warns with a thin preview line, then fires a stretched beam that lingers
class LingeringBeamPattern : public ShootingPattern {
public:
    LingeringBeamPattern(float interval = 6.0f, float warning = 1.0f, float beamDur = 2.0f, float projSpeed = 0.0f)
    : m_interval(interval), m_warning(warning), m_beamDur(beamDur), m_projSpeed(projSpeed),
      m_timer(0.0f), m_state(State::Idle) {}

    void update(float deltaTime, const sf::Vector2f& enemyPos, const sf::Vector2f& playerPos,
                std::vector<std::unique_ptr<Projectile>>& projectiles) override {
        m_timer += deltaTime;
        if (m_state == State::Idle) {
            if (m_timer >= m_interval) {
                // Start warning phase
                m_timer = 0.0f;
                m_state = State::Warning;
                std::cout << "LingeringBeamPattern: WARNING started\n";
                // compute aim angle at time of warning
                float dx = playerPos.x - enemyPos.x;
                float dy = playerPos.y - enemyPos.y;
                float angle = std::atan2(dy, dx);
                // Spawn a preview stretched projectile that lasts for warning duration
                projectiles.push_back(std::make_unique<Projectile>(enemyPos.x, enemyPos.y, angle, m_projSpeed, Projectile::Owner::Enemy, m_warning, true, true));
            }
        } else if (m_state == State::Warning) {
            if (m_timer >= m_warning) {
                // Fire beam
                m_timer = 0.0f;
                m_state = State::Firing;
                std::cout << "LingeringBeamPattern: FIRING beam\n";
                float dx = playerPos.x - enemyPos.x;
                float dy = playerPos.y - enemyPos.y;
                float angle = std::atan2(dy, dx);
                // Spawn the actual lingering beam (stretched) for beam duration
                projectiles.push_back(std::make_unique<Projectile>(enemyPos.x, enemyPos.y, angle, m_projSpeed, Projectile::Owner::Enemy, m_beamDur, true, false));
            }
        } else if (m_state == State::Firing) {
            if (m_timer >= m_beamDur) {
                // Reset
                m_timer = 0.0f;
                m_state = State::Idle;
            }
        }
    }

private:
    enum class State { Idle, Warning, Firing };
    float m_interval;
    float m_warning;
    float m_beamDur;
    float m_projSpeed;
    float m_timer;
    State m_state;
};

std::unique_ptr<ShootingPattern> makeLingeringBeamPattern(float interval, float warningDuration, float beamDuration, float projSpeed) {
    return std::make_unique<LingeringBeamPattern>(interval, warningDuration, beamDuration, projSpeed);
}
