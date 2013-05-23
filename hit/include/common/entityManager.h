#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <list>
#include <vector>
#include <ca/caPoint2d.h>
#include "common/entity.h"
#include "common/collisionEntity.h"
#include "common/graphicEntity.h"
#include "common/utils.h"
//#include "verlet/Body.h"

#define USE_SPACE_SPLIT

using std::list;
using std::vector;
using cAni::Point2f;

class CollisionChecker
{
public:
    virtual bool checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &contactInfo) = 0;
};

#ifdef USE_SPACE_SPLIT
class CollisionSpace : public NoCopy
{
public:
    enum
    {
        LeftTop = 0,
        LeftBottom,
        RightTop,
        RightBottom,
        NumSubSpaces = 4,
    };
    const int d;
    CollisionSpace(int depth, CollisionSpace* _parent = 0) : parent(_parent), d(depth)
    {
        if (depth > 0)
        {
            for (int i = 0; i < NumSubSpaces; i++)
                child[i] = new CollisionSpace(depth - 1, this);
        }
        else
        {
            for (int i = 0; i < NumSubSpaces; i++)
                child[i] = 0;
        }
    }
    virtual ~CollisionSpace()
    {
        for (int i = 0; i < NumSubSpaces; i++)
        {
            if (child[i])
                delete child[i];
        }
    }
    void setRange(const Rectf &_range)
    {
        range = _range;
        Point2f center((range.left + range.right) / 2, (range.top + range.bottom) / 2);
        if (child[LeftTop])
        {
            Rectf r = range;
            r.right = center.x;
            r.bottom = center.y;
            child[LeftTop]->setRange(r);
        }
        if (child[LeftBottom])
        {
            Rectf r = range;
            r.right = center.x;
            r.top = center.y;
            child[LeftBottom]->setRange(r);
        }
        if (child[RightTop])
        {
            Rectf r = range;
            r.left = center.x;
            r.bottom = center.y;
            child[RightTop]->setRange(r);
        }
        if (child[RightBottom])
        {
            Rectf r = range;
            r.left = center.x;
            r.top = center.y;
            child[RightBottom]->setRange(r);
        }
        // update();
    }
    void update(bool bCheckNewOnly = false);
    void addEntity(iCollisionEntity *e)
    {
        newEntities.push_back(e);
    }
    void removeEntity(iCollisionEntity *e);
    size_t checkInnerCollision(CollisionChecker &collisionChecker, iContactInfo &contactInfo);
    size_t checkCollision(iCollisionEntity *e, CollisionChecker &collisionChecker, iContactInfo &contactInfo);

    void renderDebug(float alpha);
protected:
    CollisionSpace *parent;
    CollisionSpace *child[NumSubSpaces];

    Rectf range;

    list<iCollisionEntity *> newEntities;
    list<iCollisionEntity *> entities;
};
#endif

class EntityManager : public Singleton<EntityManager>
{
public:
    EntityManager() : bModifyLock(false), collisionChecker(0), contactInfo(0)
#ifdef USE_SPACE_SPLIT
        , collisionSpace(5)
#endif
    {
        Rectf r;
        r.left = 0;
        r.right = 800;
        r.top = 0;
        r.bottom = 600;
#ifdef USE_SPACE_SPLIT
        collisionSpace.setRange(r);
#endif
    }
    virtual ~EntityManager()
    {
        clear();
    }
    void clear();
    void step(float gameTime, float deltaTime);
    void render(float gameTime, float deltaTime);
    void attach(Entity *entity);
    void dettach(Entity *entity);

    void setPlayRange(Rectf &r)
    {
#ifdef USE_SPACE_SPLIT
        collisionSpace.setRange(r);
#endif
    }
    void checkCollision();
    void setCollisionChecker(CollisionChecker *checker)
    {
        collisionChecker = checker;
    }
    void setContactInfoBuffer(iContactInfo &ci)
    {
        contactInfo = &ci;
    }
    size_t getCount() const
    {
        return entities.size();
    }
protected:
    iContactInfo &getContactInfoBuffer()
    {
        assert(contactInfo);
        return *contactInfo;
    }
    CollisionChecker *collisionChecker;

    list<Entity *> entities;
    bool bModifyLock;

    list<Entity *> newComingEntities;
    //list<CBody *> bodies;
    iContactInfo *contactInfo;
#ifdef USE_SPACE_SPLIT
    CollisionSpace collisionSpace;
#endif
};

#endif