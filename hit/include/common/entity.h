#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>
#include <curvedani.h>
#include <ca/caRect.h>
#include "utils.h"
#include "verlet/verlet.h"

using namespace std;

enum EntityInterfaceId
{
    EII_CollisionEntity,
    EII_ControlEntity,
    EII_GraphicEntity,
    EII_ViewerEntity,

    NumEntityInterfaceId,
};

class Entity : public NoCopy
{
public:
    // basic interface of entity
    class iEntity : public NoCopy
    {
    public:
        iEntity(Entity &e) : entity(e)
        {
        }
        virtual ~iEntity()
        {
        }
    protected:
        Entity &entity;
    };

    Entity() : active(true)
    {
    }
    virtual ~Entity()
    {
    }
    virtual void step(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
    bool isActive() const
    {
        return active;
    }
    virtual iEntity* getEntityInterface(EntityInterfaceId id)
    {
        id;
        return 0;
    }
protected:
    bool active;
};

#endif