#include "aiEnemyController.h"
#include "enemy.h"

AiEnemyController::AiEnemyController()
{

}

AiEnemyController::~AiEnemyController()
{

}

void AiEnemyController::think(const Point2f &playerPos)
{
    sendCommand(Enemy::CCI_SetTargetPos, &playerPos);
}