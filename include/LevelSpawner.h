#ifndef LEVELSPAWNER_H
#define LEVELSPAWNER_H

#include <memory>
#include <vector>

#include "Level.h"

class Enemy;
class Path;

std::unique_ptr<Path> makePathFor(int pathId, const std::string& start, int windowWidth, int windowHeight);

void spawnFromSpec(std::vector<std::unique_ptr<Enemy>>& enemies, const EnemySpec& spec,
                   int windowWidth, int windowHeight);

#endif
