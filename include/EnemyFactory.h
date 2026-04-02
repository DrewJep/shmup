#ifndef ENEMYFACTORY_H
#define ENEMYFACTORY_H

#include <memory>

#include "Level.h"

class Enemy;

std::unique_ptr<Enemy> makeEnemyFromSpec(const EnemySpec& spec, float x, float y, float speed = 80.0f);

#endif
