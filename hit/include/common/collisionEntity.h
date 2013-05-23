#ifndef COMMON_COLLISION_ENTITY_H
#define COMMON_COLLISION_ENTITY_H
#include "entity.h"

using cAni::Rectf;

class iContactInfo
{
public:
    virtual void Update() = 0;
};
class iCollisionEntity : public Entity::iEntity
{
public:
    iCollisionEntity(Entity &e) : Entity::iEntity(e)
    {
    }
    bool collisionEnabled() const
    {
        return bEnableCollision;
    }
    virtual unsigned long getCollisionCategory() const // 自己的类别标志位
    {
        return 0;
    }
    virtual unsigned long getCollisionBits() const // 与哪种类发生碰撞
    {
        return 0;
    }
    virtual void onCollision(iCollisionEntity &o, const iContactInfo &contactInfo)
    {
        o, contactInfo;
    }
    virtual verlet::iBody &getBody() = 0;
    virtual const verlet::iBody &getBody() const = 0;
    virtual void onAfterPhysics()
    {
        // update boundsRect here
    }
    void addIgnoredEntities(const iCollisionEntity &ie)
    {
        ignoredEntities.push_back(&ie);
    }
    const vector<const iCollisionEntity *> &getIgnoredEntities() const
    {
        return ignoredEntities;
    }
    const Rectf &getBoundsRect() const
    {
        return boundsRect;
    }
protected:
    bool bEnableCollision;
    Rectf boundsRect;
    vector<const iCollisionEntity *> ignoredEntities;
};

#endif//COMMON_COLLISION_ENTITY_H