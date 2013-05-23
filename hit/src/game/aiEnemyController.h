#ifndef GAME_AI_ENEMY_CONTROLLER_H
#define GAME_AI_ENEMY_CONTROLLER_H

#include <caPoint2d.h>
#include "common/controlEntity.h"
#include "enemy.h"

using cAni::Point2f;

class AiEnemyController : public SingleController
{
public:
    AiEnemyController();
    virtual ~AiEnemyController();

    virtual void think(const Point2f &playerPos);

    Enemy *enemy;
};

#endif//GAME_AI_ENEMY_CONTROLLER_H