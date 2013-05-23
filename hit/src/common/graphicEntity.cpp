#include <hgeCurvedani.h>
#include <caRect.h>
#include "common/graphicEntity.h"

using cAni::Rectf;

DEF_SINGLETON(RenderQueue);
Point2f RenderQueue::getViewerPos()
{
    if (viewer)
    {
        return viewer->getViewerPos() - Point2f(400, 300);
    }
    return Point2f();
}
void RenderQueue::flush()
{
    HGE *hge = hgeCreate(HGE_VERSION);
    Point2f viewerPos = getViewerPos();
    Rectf windowRange(0, 800, 0, 600);
    Rect clip;
    clip.left = 100;
    clip.right = 200;
    clip.top = 100;
    clip.bottom = 200;

    for (vector<GfxObj>::const_iterator ig = gfxobjs.begin(); ig != gfxobjs.end(); ++ig)
    {
        const GfxObj &obj = *ig;
        Point2f pos = obj.pos - viewerPos;
        // FIXME: 100 is magic number, remove it by anim clip range
        if (pos.x > windowRange.right + 100 || pos.x < windowRange.left - 100 || pos.y < windowRange.top - 100 || pos.y > windowRange.bottom + 100)
            continue;
        hge->Gfx_SetTransform(0, 0, pos.x, pos.y, obj.direction/* - viewerOrientation*/, 1, 1);
        obj.anim->render(obj.frame, 0);//&clip);
    }
    gfxobjs.clear();
    hge->Gfx_SetTransform();
    Rectf viewRange = windowRange + viewerPos;

    if (!dbgGfxObjs.empty())
    {
        for (vector<DebugGfxObj>::const_iterator idg = dbgGfxObjs.begin(); idg != dbgGfxObjs.end(); ++idg)
        {
            const DebugGfxObj &obj = *idg;
            //hge->Gfx_SetTransform(0, 0, -viewerPos.x, -viewerPos.y, 0, 1, 1);
            Rectf objRange(
                min(obj.a.x, obj.b.x),
                max(obj.a.x, obj.b.x),
                min(obj.a.y, obj.b.y),
                max(obj.a.y, obj.b.y));
            Rectf x = objRange & viewRange;
            if (x.left <= x.right && x.top <= x.bottom)
            {
                hge->Gfx_RenderLine(obj.a.x - viewerPos.x, obj.a.y - viewerPos.y,
                    obj.b.x - viewerPos.x, obj.b.y - viewerPos.y, obj.color);
            }
        }
        dbgGfxObjs.clear();
    }
    hge->Release();
}
