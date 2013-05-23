#ifndef COMMON_GRAPHIC_ENTITY_H
#define COMMON_GRAPHIC_ENTITY_H

#include <list>
#include <vector>
#include <hgeCurvedani.h>
#include <caPoint2d.h>
#include "utils.h"
#include "entity.h"

using std::list;
using std::vector;
using cAni::iAnimation;
using cAni::Point2f;


struct GfxObj
{
public:
    Point2f pos;
    float direction;
    int frame;
    const iAnimation *anim;
};
struct DebugGfxObj
{
public:
    Point2f a, b;
    DWORD color;
};
class iGraphicEntity : public Entity::iEntity
{
public:
    iGraphicEntity(Entity &e, cAni::iAnimResManager &arm) : Entity::iEntity(e), animResManager(arm)
    {
    }
    virtual void render(float gameTime, float deltaTime)
    {
        gameTime, deltaTime;
    }
protected:
    cAni::iAnimResManager &animResManager;
};

class iViewerEntity : public Entity::iEntity
{
public:
    iViewerEntity(Entity &e) : Entity::iEntity(e)
    {

    }
    virtual Point2f getViewerPos()
    {
        return Point2f();
    }
    virtual float getViewerOrientation()
    {
        return 0;
    }
};

class RenderQueue : public Singleton<RenderQueue>
{
public:
    RenderQueue() : viewer(0)
    {

    }
    void render(const Point2f &pos, float direction, int frame, const iAnimation *anim)
    {
        if (!anim)
            return;

        GfxObj obj;
        obj.pos = pos;
        obj.direction = direction;
        obj.frame = frame;
        obj.anim = anim;
        gfxobjs.push_back(obj);
    }
    void renderDebug(const Point2f &a, const Point2f &b, DWORD color)
    {
        DebugGfxObj obj;
        obj.a = a;
        obj.b = b;
        obj.color = color;
        dbgGfxObjs.push_back(obj);
    }
    void flush();
    void setViewer(Entity *entity)
    {
        if (entity)
            viewer = (iViewerEntity *)entity->getEntityInterface(EII_ViewerEntity);
        else
            viewer = 0;
    }
    void screenToGame(Point2f &pos)
    {
        pos += getViewerPos();
    }
protected:
    Point2f getViewerPos();
    vector<GfxObj> gfxobjs;
    vector<DebugGfxObj> dbgGfxObjs;
    iViewerEntity *viewer;
};

#endif//COMMON_GRAPHIC_ENTITY_H