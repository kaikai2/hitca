#include "enemy.h"
#include "game/mainGameState.h"

void Enemy::step(float gameTime, float deltaTime)
{
    gameTime, deltaTime;
    if (hp <= 0)
    {
        if (graphicEntity.isEnd(gameTime))
            active = false;
        return;
    }

    if (bHit)
    {
        bHit = false;
        Point2f dir = pos - hitFromPos;
        dir.Normalize(rand() % 10 * 20 + 50);
        pos += dir;
        lastHitTime = gameTime;
        hp--;
        if (hp <= 0)
        {
            graphicEntity.play(gameTime, 1);
        }
    }
    else
    {
        if (lastHitTime + 3 > gameTime)
            return;

        Point2f dir = targetPos - pos;
        float speed = 3 * MainGameState::getSingleton().getTimeScale();
        dir.Normalize(speed);
        pos += dir;
    }
}