#include "LevelSpawner.h"

#include <iostream>

#include "Enemy.h"
#include "Path.h"
#include "ShootingPattern.h"

std::unique_ptr<Path> makePathFor(int pathId, const std::string& start, int windowWidth, int windowHeight) {
    std::vector<sf::Vector2f> waypoints;
    float midY = windowHeight / 2.0f;
    float leftX = windowWidth * 0.12f;
    float rightX = windowWidth * 0.88f;
    float centerX = windowWidth * 0.5f;

    if (pathId == 1) {
        float sx = centerX;
        if (start == "left") sx = leftX;
        if (start == "right") sx = rightX;
        waypoints.push_back({sx, midY - 40.0f});
        waypoints.push_back({sx, midY + 40.0f});
        return std::make_unique<Path>(waypoints, 80.0f, true);
    }
    if (pathId == 2) {
        waypoints = {{rightX, midY}, {centerX, midY - 60.0f}, {leftX, midY}, {centerX, midY + 60.0f}};
        return std::make_unique<Path>(waypoints, 80.0f, true);
    }
    if (pathId == 3) {
        if (start == "left") {
            waypoints = {{leftX, midY - 30.0f}, {rightX, midY + 30.0f}};
        } else if (start == "right") {
            waypoints = {{rightX, midY - 30.0f}, {leftX, midY + 30.0f}};
        } else {
            waypoints = {{centerX - 60.0f, midY}, {centerX + 60.0f, midY}};
        }
        return std::make_unique<Path>(waypoints, 120.0f, true);
    }

    waypoints.push_back({centerX, midY});
    return std::make_unique<Path>(waypoints, 50.0f, true);
}

void spawnFromSpec(std::vector<std::unique_ptr<Enemy>>& enemies, const EnemySpec& spec,
                   int windowWidth, int windowHeight) {
    std::cout << "spawnFromSpec: spawning -> hp=" << spec.hp
              << " type='" << spec.type << "'"
              << " shot=" << spec.shot
              << " path=" << spec.path
              << " start='" << spec.start << "'\n";

    float x = windowWidth * 0.5f;
    float y = windowHeight * 0.5f;
    if (spec.start == "left") x = windowWidth * 0.12f;
    else if (spec.start == "right") x = windowWidth * 0.88f;

    y += (spec.path % 3 - 1) * 20.0f;

    Enemy::Type et = (spec.type == 'T') ? Enemy::Type::Tank : Enemy::Type::UFO;
    auto enemy = std::make_unique<Enemy>(x, y, 80.0f, et);
    enemy->setHealth(spec.hp);

    enemy->setPath(makePathFor(spec.path, spec.start, windowWidth, windowHeight));

    if (spec.shot == 1) {
        enemy->setShootingPattern(makeDirectAtPlayerPattern(1.5f, 220.0f, 400.0f, false));
    } else if (spec.shot == 2) {
        enemy->setShootingPattern(makeRadialPattern(8, 2.5f, 160.0f));
    }

    enemies.push_back(std::move(enemy));
}
