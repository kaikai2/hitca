#ifndef MAINGAMESTATE_H
#define MAINGAMESTATE_H

#include <list>

#include <hgegui.h>
#include <hgecolor.h>
#include <curvedani.h>
#include <hgeCurvedAni.h>
#include "common/gamestate.h"
#include "common/entity.h"
#include "common/controlEntity.h"
#include "common/utils.h"
#include "common/entitymanager.h"
#include "common/graphicEntity.h"
#include "verlet/verlet.h"

using namespace std;

class hgeFont;
class hgeSprite;
class AiEnemyController;
class Enemy;

class HitContactInfo : public iContactInfo
{
public:
    verlet::Contact *getData()
    {
        return contact;
    }
    void setNumber(size_t num)
    {
        this->num = num;
    }
    virtual void Update()
    {
        static Point2f off[4] =
        {
            Point2f(-3, -3),
            Point2f(3, -3),
            Point2f(3, 3),
            Point2f(-3, 3),
        };
        for(size_t i = 0; i < num; i++)
        {
            RenderQueue::getSingleton().renderDebug(contact[i].m_xContacts[0] + off[0], contact[i].m_xContacts[1] + off[2], ARGB(255, 0, 255, 0));
            RenderQueue::getSingleton().renderDebug(contact[i].m_xContacts[0] + off[1], contact[i].m_xContacts[1] + off[3], ARGB(255, 0, 255, 0));
            contact[i].ResolveOverlap();
            contact[i].ResolveCollision();
        }
    }
protected:
    verlet::Contact contact[8];
    size_t num;
};

class HitCollisionChecker : public CollisionChecker
{
public:
    bool checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo);

    int checkCount;
//protected:
    float deltaTime;
};

class MainGameState : public GameState, public Singleton<MainGameState>
{
public:
    MainGameState() : hge(0), gui(0), font(0), animResManager(0), system(0)
    {
    }
    virtual ~MainGameState();

    virtual void OnEnter();
    virtual void OnLeave();
    virtual void OnFrame();
    virtual void OnRender();

    Enemy *getNearestEnemy(const Point2f &pos);

    void pauseGame(float time, float scale = 0.5f);
    float getTimeScale() const
    {
        if (pauseTime > 0)
            return timeScale;
        return 1.0f;
    }
protected:
    void ProcessControl(const hgeInputEvent &event);
    void GenerateEnemy();

    HGE *hge;
    HTEXTURE texGui;
    hgeGUI *gui;
    hgeFont *font;
    cAni::iAnimResManager *animResManager;
    hgeCurvedAniSystem *system;

    SingleController controller;
    list<AiEnemyController*> enemies;

    HitCollisionChecker collisionChecker;
    HitContactInfo contactInfo;
    RenderQueue renderQueue;

    float lastGenTime;
    float pauseTime;
    float timeScale;
};

#endif
