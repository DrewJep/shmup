#include "EnemyFactory.h"

#include "Enemy.h"
#include "TankEnemy.h"
#include "UfoEnemy.h"

std::unique_ptr<Enemy> makeEnemyFromSpec(const EnemySpec& spec, float x, float y, float speed) {
    if (spec.type == 'T') {
        return std::make_unique<TankEnemy>(x, y, speed);
    }
    return std::make_unique<UfoEnemy>(x, y, speed);
}
