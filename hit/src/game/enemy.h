#ifndef GAME_ENEMY_H
#define GAME_ENEMY_H

#include <curvedani.h>
#include <caPoint2d.h>

#include "common/entity.h"
#include "common/graphicEntity.h"
#include "common/controlEntity.h"
#include "common/collisionEntity.h"
#include "verlet/verlet.h"
#include "collisionDef.h"

using cAni::Point2f;
class Enemy : public Entity
{
public:
    enum ControlCommandId
    {
        CCI_Attack,
        CCI_Defence,
        CCI_Special,
        CCI_SetTargetPos,
        NumControlCommandId,
    };
    class ControlEntity : public iControlEntity
    {
    public:
        ControlEntity(Enemy &p) : iControlEntity(p)
        {
        }
        virtual void onControllerCommand(int command, const void *pData)
        {
            switch(command)
            {
            case CCI_Attack:
                break;
            case CCI_Defence:
                break;
            case CCI_Special:
                break;
            case CCI_SetTargetPos:
                getEntity().targetPos = *(const Point2f *)pData;
                break;
            default:
                assert(0);
                break;
            }
        }
    protected:
        Enemy &getEntity()
        {
            return *(Enemy *)&entity;
        }
        const Enemy &getEntity() const
        {
            return *(const Enemy *)&entity;
        }
    };
    Enemy(cAni::iAnimResManager &arm) :
        controlEntity(*this),
        graphicEntity(*this, arm),
        collisionEntity(*this)
    {
        bHit = false;
        hp = 3;

    }
    virtual iEntity* getEntityInterface(EntityInterfaceId id)
    {
        switch(id)
        {
        case EII_CollisionEntity:
            break;
        case EII_ControlEntity:
            return &controlEntity;
        case EII_GraphicEntity:
            return &graphicEntity;
        case EII_ViewerEntity:
            break;
        }
        return 0;
    }

    Point2f pos;
    float orientation;
    float lastHitTime;

    bool bHit;
    Point2f hitFromPos;

    Point2f targetPos; // attack to which position
    int hp;

    void init(const char *xml, const char *xml2)
    {
        graphicEntity.init(xml, xml2);
    }
    void hit(const Point2f &fromPos)
    {
        hitFromPos = fromPos;
        bHit = true;
    }
    virtual void step(float gameTime, float deltaTime);
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Enemy &p, cAni::iAnimResManager &arm) : iGraphicEntity(p, arm), anim(0)
        {
        }
        virtual ~GraphicEntity()
        {
            iSystem::GetInstance()->release(anim);
        }
        void init(const char *xml, const char *xml2)
        {
            if (!anim)
            {
                anim = iSystem::GetInstance()->createAnimation(2);
            }
            anim->setAnimData(animResManager.getAnimData(xml), 0);
            anim->setAnimData(animResManager.getAnimData(xml2), 1);
        }
        void play(float gameTime, int i)
        {
            anim->startAnim(int(gameTime * 60), i);
        }
        bool isEnd(float gameTime)
        {
            return anim->checkEnd(int(gameTime * 60));
        }
        virtual void render(float gameTime, float deltaTime)
        {
            deltaTime;
            RenderQueue::getSingleton().render(getEntity().pos, getEntity().orientation, int(gameTime * 60), anim);
        }
    protected:
        Enemy &getEntity()
        {
            return *(Enemy *)&entity;
        }
        const Enemy &getEntity() const
        {
            return *(const Enemy *)&entity;
        }
        cAni::iAnimation *anim;
    };
    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Enemy &p) : iCollisionEntity(p)
        {
            verlet::CircleShape *shape = new verlet::CircleShape(body, 18);
            body.SetShape(shape);
        }
        virtual ~CollisionEntity()
        {
            if (body.GetShape())
            {
                delete body.GetShape();
                body.SetShape(0);
            }
        }

        virtual unsigned long getCollisionCategory() const // 自己的类别标志位
        {
            return CC_Enemy;
        }
        virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
        {
            return CB_Enemy;
        }
        virtual void onCollision(iCollisionEntity &o, const iContactInfo &contactInfo)
        {
            o, contactInfo;
        }
        virtual verlet::iBody &getBody()
        {
            return body;
        }
        virtual const verlet::iBody &getBody() const
        {
            return body;
        }
        virtual void onAfterPhysics()
        {
            // update boundsRect here
            getEntity().pos = getBody().getPosition();
        }
        Enemy &getEntity()
        {
            return *(Enemy *)&entity;
        }
        const Enemy &getEntity() const
        {
            return *(const Enemy *)&entity;
        }

    protected:
        verlet::Body body;
    };

protected:
    ControlEntity controlEntity;
    GraphicEntity graphicEntity;
    CollisionEntity collisionEntity;
};

#endif//GAME_ENEMY_H