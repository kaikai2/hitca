#include "player.h"
#include "enemy.h"
#include "game/mainGameState.h"

void Player::Attack()
{
    Enemy *pEnemy = MainGameState::getSingleton().getNearestEnemy(pos);
    if (!pEnemy)
        return;

    Point2f dir = pEnemy->pos - pos;
    float len = dir.Length();
    if (len > attackRange)
    {
        MainGameState::getSingleton().pauseGame(0.5f, 2.0f);
        return;
    }

    //bAttack = true;
    dir.Normalize();

    pos = pEnemy->pos - dir * 20 * MainGameState::getSingleton().getTimeScale();
    pEnemy->hit(pos);

    MainGameState::getSingleton().pauseGame(0.5f, 0.1f);
}

void Player::step(float gameTime, float deltaTime)
{
    gameTime, deltaTime;

    Point2f dir = Point2f(400, 300) - pos;
    float speed = 1 * MainGameState::getSingleton().getTimeScale();
    dir.Normalize(speed);
    pos += dir;
}
void Player::GraphicEntity::render(float gameTime, float deltaTime)
{
    gameTime;
    frameTime += deltaTime * MainGameState::getSingleton().getTimeScale();
    RenderQueue::getSingleton().render(getEntity().pos, getEntity().orientation, int(frameTime * 60), anim);
}