// disable stl debugging in debug mode also
//#define _HAS_ITERATOR_DEBUGGING 0
//#define _SECURE_SCL 0

#include <vector>
#include <algorithm>
#include "common/entityManager.h"
#include "common/profile.h"
using std::vector;
using std::find;


DEF_SINGLETON(EntityManager);

void EntityManager::clear()
{
    bModifyLock = true;
    for (list<Entity*>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
    {
        delete *ie;
    }
    //this->bodies.clear();
    entities.clear();
    bModifyLock = false;
}

void EntityManager::step(float gameTime, float deltaTime)
{
    list<Entity*>::iterator ie;
    bModifyLock = true;
    if (!newComingEntities.empty())
    {
        PROFILE_CHECKPOINT(_addNewEntity);
        entities.insert(entities.end(), newComingEntities.begin(), newComingEntities.end());
        for (ie = newComingEntities.begin(); ie != newComingEntities.end(); ++ie)
        {
            iCollisionEntity *collEntity = (iCollisionEntity *)(*ie)->getEntityInterface(EII_CollisionEntity);
            if (collEntity)
            {
                //this->bodies.push_back(&collEntity->getBody());
#ifdef USE_SPACE_SPLIT
                collisionSpace.addEntity(collEntity);
#endif
            }
            //this->entities.push_back(entity);
        }
        newComingEntities.clear();
        PROFILE_ENDPOINT();
    }
    PROFILE_CHECKPOINT(_stepAllEntities);
    for (ie = entities.begin(); ie != entities.end(); ++ie)
    {
        (*ie)->step(gameTime, deltaTime);
    }
    for (ie = entities.begin(); ie != entities.end();)
    {
        if (!(*ie)->isActive())
        {
            iCollisionEntity *collEntity = (iCollisionEntity *)(*ie)->getEntityInterface(EII_CollisionEntity);
            if (collEntity)
            {
                //this->bodies.remove(&collEntity->getBody());
#ifdef USE_SPACE_SPLIT
                collisionSpace.removeEntity(collEntity);
#endif
            }
            delete *ie;
            ie = entities.erase(ie);
        }
        else
            ++ie;
    }
    PROFILE_CHECKPOINT(_CheckCollision);
//    bodies.Update(deltaTime);
    checkCollision();
    PROFILE_CHECKPOINT(_call_onAfterPhysics);
    for (ie = entities.begin(); ie != entities.end(); ++ie)
    {
        iCollisionEntity *ce = (iCollisionEntity *)(*ie)->getEntityInterface(EII_CollisionEntity);
        if (ce)
        {
            ce->onAfterPhysics();
        }
    }
    bModifyLock = false;
    PROFILE_ENDPOINT();
}

void EntityManager::render(float gameTime, float deltaTime)
{
    bModifyLock = true;
    for (list<Entity*>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
    {
        iGraphicEntity *graphicEntity = (iGraphicEntity *)(*ie)->getEntityInterface(EII_GraphicEntity);
        if (graphicEntity)
        {
            graphicEntity->render(gameTime, deltaTime);
        }
    }
#ifdef USE_SPACE_SPLIT
#ifdef _DEBUG
    this->collisionSpace.renderDebug(0.8f);
#endif
#endif
    bModifyLock = false;
}

void EntityManager::attach(Entity *entity)
{
    assert(entity);
    if (!bModifyLock)
    {
        iCollisionEntity *collEntity = (iCollisionEntity *)entity->getEntityInterface(EII_CollisionEntity);
        if (collEntity)
        {
            //this->bodies.push_back(&collEntity->getBody());
#ifdef USE_SPACE_SPLIT
            collisionSpace.addEntity(collEntity);
#endif
        }
        this->entities.push_back(entity);
    }
    else
    {
        this->newComingEntities.push_back(entity);
    }
}

void EntityManager::dettach(Entity *entity)
{
    assert(!bModifyLock);
    iCollisionEntity *collEntity = (iCollisionEntity *)entity->getEntityInterface(EII_CollisionEntity);
    if (collEntity)
    {
        //this->bodies.remove(&collEntity->getBody());
#ifdef USE_SPACE_SPLIT
        collisionSpace.removeEntity(collEntity);
#endif
    }
    this->entities.remove(entity);
}

void EntityManager::checkCollision()
{
    if (!collisionChecker)
        return;

    iContactInfo &contactInfo = getContactInfoBuffer();
    bModifyLock = true;
#ifdef USE_SPACE_SPLIT
    for (int i = 0; i < 1; i++)
    {
        PROFILE_CHECKPOINT(__collisionSpaceUpdate);
        collisionSpace.update();
        PROFILE_CHECKPOINT(__checkInnerCollision);
        if (collisionSpace.checkInnerCollision(*collisionChecker, contactInfo) == 0)
            break;

        for (list<Entity*>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
        {
            iCollisionEntity *ce = (iCollisionEntity *)(*ie)->getEntityInterface(EII_CollisionEntity);
            if (ce)
            {
                ce->onAfterPhysics();
            }
        }
    }
    PROFILE_ENDPOINT();
#else
    vector<iCollisionEntity*> collisionEntities;
    for (list<Entity*>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
    {
        iCollisionEntity *ce = (iCollisionEntity *)(*ie)->getEntityInterface(EII_CollisionEntity);
        if (ce)
        {
            collisionEntities.push_back(ce);
        }
    }

    // naive o^2 test algorithm
    for (vector<iCollisionEntity*>::iterator ice = collisionEntities.begin(); ice != collisionEntities.end(); ++ice)
    {
        iCollisionEntity *e = *ice;
        unsigned long category = e->getCollisionCategory();
        unsigned long collision = e->getCollisionBits();
        if ((category | collision) == 0)
            continue;

        const vector<const iCollisionEntity*> &ies1 = e->getIgnoredEntities();

        for (vector<iCollisionEntity*>::iterator ice2 = ice + 1; ice2 != collisionEntities.end(); ++ice2)
        {
            iCollisionEntity *e2 = *ice2;
            unsigned long category2 = e2->getCollisionCategory();
            unsigned long collision2 = e2->getCollisionBits();

            if (!(category & collision2) && !(category2 & collision))
                continue;

            if (find(ies1.begin(), ies1.end(), e2) != ies1.end())
                continue;

            const vector<const iCollisionEntity*> &ies2 = e2->getIgnoredEntities();

            if (find(ies2.begin(), ies2.end(), e) != ies2.end())
                continue;

            if (collisionChecker->checkCollision(e, e2, contactInfo))
            {
                contactInfo.Update();
                e->onCollision(*e2, contactInfo);
                e2->onCollision(*e, contactInfo);
            }
        }
    }
#endif
    bModifyLock = false;
}
#ifdef USE_SPACE_SPLIT
size_t CollisionSpace::checkInnerCollision(CollisionChecker &collisionChecker, iContactInfo &contactInfo)
{
    assert(newEntities.empty());
    size_t collisionCount = 0;
    for (list<iCollisionEntity *>::iterator ie1 = entities.begin(); ie1 != entities.end(); ++ie1)
    {
        iCollisionEntity *e = *ie1;
        const unsigned long category = e->getCollisionCategory();
        const unsigned long collision = e->getCollisionBits();
        if ((category | collision) == 0)
            continue;

        const vector<const iCollisionEntity*> &ies1 = e->getIgnoredEntities();

        list<iCollisionEntity *>::iterator ie2 = ie1;
        for (++ie2; ie2 != entities.end(); ++ie2)
        {
            iCollisionEntity *e2 = *ie2;
            const unsigned long category2 = e2->getCollisionCategory();
            const unsigned long collision2 = e2->getCollisionBits();

            if (!(category & collision2) && !(category2 & collision))
                continue;

            if (find(ies1.begin(), ies1.end(), e2) != ies1.end())
                continue;

            const vector<const iCollisionEntity*> &ies2 = e2->getIgnoredEntities();

            if (find(ies2.begin(), ies2.end(), e) != ies2.end())
                continue;

            if (collisionChecker.checkCollision(e, e2, contactInfo))
            {
                collisionCount++;
                contactInfo.Update();
                e->onCollision(*e2, contactInfo);
                e2->onCollision(*e, contactInfo);
            }
        }

        const Rectf &r = e->getBoundsRect();
        Point2f center((range.left + range.right) / 2, (range.top + range.bottom) / 2);
        if (child[LeftTop])
        {
            if (r.left < center.x && r.top < center.y)
                collisionCount += child[LeftTop]->checkCollision(e, collisionChecker, contactInfo);
        }
        if (child[LeftBottom])
        {
            if (r.left < center.x && r.bottom > center.y)
                collisionCount += child[LeftBottom]->checkCollision(e, collisionChecker, contactInfo);
        }
        if (child[RightTop])
        {
            if (r.right > center.x && r.top < center.y)
                collisionCount += child[RightTop]->checkCollision(e, collisionChecker, contactInfo);
        }
        if (child[RightBottom])
        {
            if (r.right > center.x && r.bottom > center.y)
                collisionCount += child[RightBottom]->checkCollision(e, collisionChecker, contactInfo);
        }
    }
    if (child[LeftTop])
        collisionCount += child[LeftTop]->checkInnerCollision(collisionChecker, contactInfo);
    if (child[LeftBottom])
        collisionCount += child[LeftBottom]->checkInnerCollision(collisionChecker, contactInfo);
    if (child[RightTop])
        collisionCount += child[RightTop]->checkInnerCollision(collisionChecker, contactInfo);
    if (child[RightBottom])
        collisionCount += child[RightBottom]->checkInnerCollision(collisionChecker, contactInfo);
    
    return collisionCount;
}

size_t CollisionSpace::checkCollision(iCollisionEntity *e, CollisionChecker &collisionChecker, iContactInfo &contactInfo)
{
    assert(e);
    const unsigned long category = e->getCollisionCategory();
    const unsigned long collision = e->getCollisionBits();
    if ((category | collision) == 0)
        return 0;
    
    size_t collisionCount = 0;
    const vector<const iCollisionEntity*> &ies1 = e->getIgnoredEntities();

    for (list<iCollisionEntity *>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
    {
        iCollisionEntity *e2 = *ie;
        const unsigned long category2 = e2->getCollisionCategory();
        const unsigned long collision2 = e2->getCollisionBits();

        if (!(category & collision2) && !(category2 & collision))
            continue;

        if (find(ies1.begin(), ies1.end(), e2) != ies1.end())
            continue;

        const vector<const iCollisionEntity*> &ies2 = e2->getIgnoredEntities();

        if (find(ies2.begin(), ies2.end(), e) != ies2.end())
            continue;

        if (collisionChecker.checkCollision(e, e2, contactInfo))
        {
            collisionCount++;
            contactInfo.Update();
            e->onCollision(*e2, contactInfo);
            e2->onCollision(*e, contactInfo);
        }
    }

    const Rectf &r = e->getBoundsRect();
    Point2f center((range.left + range.right) / 2, (range.top + range.bottom) / 2);
    if (r.left < center.x && r.top < center.y && child[LeftTop])
        collisionCount += child[LeftTop]->checkCollision(e, collisionChecker, contactInfo);
    if (r.left < center.x && r.bottom > center.y && child[LeftBottom])
        collisionCount += child[LeftBottom]->checkCollision(e, collisionChecker, contactInfo);
    if (r.right > center.x && r.top < center.y && child[RightTop])
        collisionCount += child[RightTop]->checkCollision(e, collisionChecker, contactInfo);
    if (r.right > center.x && r.bottom > center.y && child[RightBottom])
        collisionCount += child[RightBottom]->checkCollision(e, collisionChecker, contactInfo);

    return collisionCount;
}


void CollisionSpace::update(bool bCheckNewOnly/* = false*/)
{
    Point2f center((range.left + range.right) / 2, (range.top + range.bottom) / 2);
    list<iCollisionEntity *>::iterator ie;
    //size_t from = 0;
    if (bCheckNewOnly)
    {
        ie = entities.end();
        //from = entities.size();
    }
    else
    {
        ie = entities.begin();
    }
    entities.insert(entities.end(), newEntities.begin(), newEntities.end());
    newEntities.clear();

    bool checkParent = false;
    //bool checkChild[NumSubSpaces] = {false, };
    //ie = entities.begin() + from;
    while(ie != entities.end())
    {
        const Rectf &r = (*ie)->getBoundsRect();
        if (parent && (r.left < range.left || r.right > range.right || r.top < range.top || r.bottom > range.bottom))
        {
            parent->addEntity(*ie);
            ie = entities.erase(ie);
            //swap(*ie, entities.back());entities.pop_back();
            checkParent = true;
            continue;
        }
        if (child[LeftTop])
        {
            if (r.right < center.x && r.bottom < center.y)
            {
                child[LeftTop]->addEntity(*ie);
                ie = entities.erase(ie);
                //swap(*ie, entities.back());entities.pop_back();
                //checkChild[LeftTop] = true;
                continue;
            }
        }
        if (child[LeftBottom])
        {
            if (r.right < center.x && r.top > center.y)
            {
                child[LeftBottom]->addEntity(*ie);
                ie = entities.erase(ie);
                //swap(*ie, entities.back());entities.pop_back();
                //checkChild[LeftBottom] = true;
                continue;
            }
        }
        if (child[RightTop])
        {
            if (r.left > center.x && r.bottom < center.y)
            {
                child[RightTop]->addEntity(*ie);
                ie = entities.erase(ie);
                //swap(*ie, entities.back());entities.pop_back();
                //checkChild[RightTop] = true;
                continue;
            }
        }
        if (child[RightBottom])
        {
            if (r.left > center.x && r.top > center.y)
            {
                child[RightBottom]->addEntity(*ie);
                ie = entities.erase(ie);
                //swap(*ie, entities.back());entities.pop_back();
                //checkChild[RightBottom] = true;
                continue;
            }
        }

        ++ie;
    }

    if (checkParent)
    {
        assert(parent);
        parent->update(true);
    }
    for (int i = 0; i < NumSubSpaces; i++)
    {
        if (child[i])
        {
            //assert(child[i]);
            child[i]->update(bCheckNewOnly);
        }
    }
}

void CollisionSpace::removeEntity(iCollisionEntity *e)
{
    //entities.erase(remove(entities.begin(), entities.end(), e), entities.end());
    entities.remove(e);
    //newEntities.erase(remove(newEntities.begin(), newEntities.end(), e), newEntities.end());
    newEntities.remove(e);
    for (int i = 0; i < NumSubSpaces; i++)
    {
        if (child[i])
            child[i]->removeEntity(e);
    }
}
#include <hge.h>
void CollisionSpace::renderDebug(float alpha)
{
    //HGE *hge = hgeCreate(HGE_VERSION);
    //hge->Gfx_SetTransform();
    if (child[LeftTop])
    {
        const Rectf &r = child[LeftTop]->range;
        RenderQueue::getSingleton().renderDebug(Point2f(r.left, r.bottom), Point2f(r.right, r.bottom), ARGB(255 * alpha, 255, 255, 255));
        RenderQueue::getSingleton().renderDebug(Point2f(r.right, r.top), Point2f(r.right, r.bottom), ARGB(255 * alpha, 255, 255, 255));

        child[LeftTop]->renderDebug(alpha * 0.8f);
        if (child[LeftBottom])
        child[LeftBottom]->renderDebug(alpha * 0.8f);
        if (child[RightTop])
        child[RightTop]->renderDebug(alpha * 0.8f);
        if (child[RightBottom])
        child[RightBottom]->renderDebug(alpha * 0.8f);
    }
    Point2f center((range.left + range.right) / 2, (range.top + range.bottom) / 2);

    for (list<iCollisionEntity *>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
    {
        const Rectf &r = (*ie)->getBoundsRect();
        RenderQueue::getSingleton().renderDebug(center, Point2f(r.left, r.top), ARGB(255 * alpha, 255, 0, 0));
    }
    // hge->Release();
}
#endif