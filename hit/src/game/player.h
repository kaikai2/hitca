#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include <curvedani.h>
#include <caPoint2d.h>

#include "common/entity.h"
#include "common/graphicEntity.h"
#include "common/controlEntity.h"
#include "common/collisionEntity.h"
#include "verlet/verlet.h"
#include "collisionDef.h"

using cAni::Point2f;

class Player : public Entity
{
public:
    enum ControlCommandId
    {
        CCI_Attack,
        CCI_Defence,
        CCI_Special,
        NumControlCommandId,
    };

    class ControlEntity : public iControlEntity
    {
    public:
        ControlEntity(Player &p) : iControlEntity(p)
        {
        }
        virtual void onControllerCommand(int command, const void *pData)
        {
            switch(command)
            {
            case CCI_Attack:
                if (pData)
                    getEntity().Attack();
                break;
            case CCI_Defence:
                getEntity().setDefence(pData ? true : false);
                break;
            case CCI_Special:
                break;
            default:
                assert(0);
                break;
            }
        }
    protected:
        Player &getEntity()
        {
            return *(Player *)&entity;
        }
        const Player &getEntity() const
        {
            return *(const Player *)&entity;
        }
    };
    Player(cAni::iAnimResManager &arm) :
        controlEntity(*this),
        graphicEntity(*this, arm),
        collisionEntity(*this)
    {
        bDefence = false;
        attackRange = 100;
       // bAttack = false;
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
    bool bDefence;
    float attackRange;
    //bool bAttack;

    void init(const char *xml)
    {
        graphicEntity.init(xml);
    }
    void Attack();
    void setDefence(bool bFlag)
    {
        bDefence = bFlag;
    }
    virtual void step(float gameTime, float deltaTime);
protected:
    class GraphicEntity : public iGraphicEntity
    {
    public:
        GraphicEntity(Player &p, cAni::iAnimResManager &arm) : iGraphicEntity(p, arm), anim(0)
        {
            frameTime = 0;
        }
        virtual ~GraphicEntity()
        {
            iSystem::GetInstance()->release(anim);
        }
        void init(const char *xml)
        {
            if (!anim)
            {
                anim = iSystem::GetInstance()->createAnimation();
            }
            anim->setAnimData(animResManager.getAnimData(xml), 0);
        }
        virtual void render(float gameTime, float deltaTime);
    protected:
        Player &getEntity()
        {
            return *(Player *)&entity;
        }
        const Player &getEntity() const
        {
            return *(const Player *)&entity;
        }
        float frameTime;
        cAni::iAnimation *anim;
    };
    class CollisionEntity : public iCollisionEntity
    {
    public:
        CollisionEntity(Player &p) : iCollisionEntity(p)
        {
            verlet::CircleShape *shape = new verlet::CircleShape(body, 49);
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
    protected:
        Player &getEntity()
        {
            return *(Player *)&entity;
        }
        const Player &getEntity() const
        {
            return *(const Player *)&entity;
        }
        
        verlet::Body body;
    };

    ControlEntity controlEntity;
    GraphicEntity graphicEntity;
    CollisionEntity collisionEntity;
};

#endif//GAME_PLAYER_H