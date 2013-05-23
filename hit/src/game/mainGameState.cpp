#define USE_PROFILER 1
#include <cmath>
#include <cassert>
#include <cctype>

#include <hgefont.h>
#include <hgesprite.h>
#include <hgeguictrls.h>
#include <hgecolor.h>

#include "game/maingamestate.h"
#include "common/entity.h"
#include "common/entitymanager.h"
#include "common/graphicEntity.h"
#include "common/profile.h"

#include "player.h"
#include "enemy.h"
#include "aiEnemyController.h"


#ifndef SAFE_DELETE
#define SAFE_DELETE(a) if (!a);else {delete a; a = 0;}
#endif

enum GUI_ID
{
    /// GUI main
    GID_BtnRand = 1,
    GID_BtnStart,

    NumGUIId,
};

bool HitCollisionChecker::checkCollision(iCollisionEntity *a, iCollisionEntity *b, iContactInfo &_contactInfo)
{
    // m_xBodies[i].Collide(m_xBodies[j], dt);
    HitContactInfo &contactInfo = (HitContactInfo &)_contactInfo;
    size_t n = a->getBody().Collide(b->getBody(), deltaTime, contactInfo.getData(), 8);
    contactInfo.setNumber(n);
    checkCount++;
    return n > 0;
}
bool dbg_UseFriction = true;

DEF_SINGLETON(MainGameState);
MainGameState::~MainGameState()
{
    assert(!this->gui);
    assert(!this->hge);
}
Player *gPlayer = 0;
void MainGameState::OnEnter()
{
    this->hge = hgeCreate(HGE_VERSION);
    this->font = new hgeFont("data/font.fnt");
    this->font->SetColor(ARGB(255, 0, 255, 0));

    this->system = new hgeCurvedAniSystem;
    this->animResManager = iSystem::GetInstance()->createAnimResManager();

    this->texGui = hge->Texture_Load("data/ui.png");
    this->gui = new hgeGUI;
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnRand, 10, 10, 36, 19, texGui, 0, 0));
    this->gui->AddCtrl(new hgeGUIButton(GID_BtnStart, 50, 10, 39, 19, texGui, 0, 19));
    Rectf playRange;
    playRange.left = -2000;
    playRange.right = 2000;
    playRange.top = -2000;
    playRange.bottom = 2000;
    EntityManager::getSingleton().setPlayRange(playRange);
    EntityManager::getSingleton().setCollisionChecker(&collisionChecker);
    EntityManager::getSingleton().setContactInfoBuffer(contactInfo);
    EntityManager::getSingleton().clear();

    // add tank
    Player *player = new Player(*animResManager);
    gPlayer = player;
    player->pos = Point2f(400, 300);
    player->init("data/player/player.xml");
    //RenderQueue::getSingleton().setViewer(player);

    ((Player::ControlEntity *)player->getEntityInterface(EII_ControlEntity))->attach(controller);

    EntityManager::getSingleton().attach(player);

    lastGenTime = hge->Timer_GetTime();
    pauseTime = 0;
}

void MainGameState::OnLeave()
{
    EntityManager::getSingleton().setCollisionChecker(0);
    EntityManager::getSingleton().clear();
    SAFE_DELETE(this->gui);
    SAFE_DELETE(this->font);
    iSystem::GetInstance()->release(this->animResManager);
    SAFE_DELETE(this->system);
    if (this->texGui)
    {
        this->hge->Texture_Free(this->texGui);
        this->texGui = 0;
    }
    this->hge->Release();
    this->hge = 0;
}

void MainGameState::pauseGame(float time, float scale)
{
    pauseTime = time;
    timeScale = scale;
}

void MainGameState::ProcessControl(const hgeInputEvent &event)
{
    switch(event.type)
    {
    case INPUT_KEYDOWN:
        switch(event.key)
        {
        case HGEK_A:
            this->controller.sendCommand(Player::CCI_Attack, "1");
            break;
        case HGEK_S:
            this->controller.sendCommand(Player::CCI_Defence, "1");
            break;
        case HGEK_D:
            this->controller.sendCommand(Player::CCI_Special, "1");
            break;
        }
        break;
    case INPUT_KEYUP:
        switch(event.key)
        {
        case HGEK_A:
            this->controller.sendCommand(Player::CCI_Attack, 0);
            break;
        case HGEK_S:
            this->controller.sendCommand(Player::CCI_Defence, 0);
            break;
        case HGEK_D:
            this->controller.sendCommand(Player::CCI_Special, 0);
            break;
        }
        break;
    case INPUT_MBUTTONDOWN:
    case INPUT_MBUTTONUP:
    case INPUT_MOUSEMOVE:
    case INPUT_MOUSEWHEEL:
        break;
    }
}

void MainGameState::GenerateEnemy()
{
    Enemy *enemy = new Enemy(*this->animResManager);
    EntityManager::getSingleton().attach(enemy);

    AiEnemyController *aec = 0;
    for (list<AiEnemyController *>::iterator ie = enemies.begin(); ie != enemies.end(); ++ie)
    {
        if (!(*ie)->isAttached())
        {
            aec = *ie;
            break;
        }
    }
    if (aec == 0)
    {
        aec = new AiEnemyController;
        enemies.push_back(aec);
    }
    aec->attach(*(iControlEntity *)enemy->getEntityInterface(EII_ControlEntity));
    aec->enemy = enemy;
    if (rand() % 2)
    {
        enemy->init("data/enemy/enemy.xml", "data/enemy/explode.xml");
        enemy->hp = 3;
    }
    else
    {
        enemy->init("data/enemy/enemy2.xml", "data/enemy/explode.xml");
        enemy->hp = 5;
    }
    int w = 800 + 100;
    int h = 600 + 100;
    int r = rand() % ((w +h) * 2);
    if (r < w)
        enemy->pos = Point2f(r, -50);
    else if (r < w + h)
        enemy->pos = Point2f(-50, r - w);
    else if (r < w + w + h)
        enemy->pos = Point2f(r - w - h, w - 50);
    else
        enemy->pos = Point2f(h - 50, r - w - w - h);
}

int checkCount;
void MainGameState::OnFrame()
{
    PROFILE_CHECKPOINT(Frame);
    if (this->hge->Input_GetKeyState(HGEK_ESCAPE))
    {
        // RequestState("mainmenu");
        RequestState("exit");
    }
    hgeInputEvent event;
    while(this->hge->Input_GetEvent(&event))
    {
        ProcessControl(event);
    }

    //float fCurTime = this->hge->Timer_GetTime();

    int id = this->gui->Update(this->hge->Timer_GetDelta());
    switch(id)
    {
    case GID_BtnRand:
        //gTank->SetCannon(*Cannon::GetData(rand() % 3));
        break;
    case GID_BtnStart:
        //gTank->SetGun(*Gun::GetData(rand() % 2));
        break;
    }
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
    if (pauseTime > 0)
        pauseTime -= deltaTime;

    collisionChecker.deltaTime = deltaTime;
    size_t restCount = 0;
    for (list<AiEnemyController *>::iterator ie = enemies.begin(); ie != enemies.end(); ++ie)
    {
        if ((*ie)->isAttached())
        {
            (*ie)->think(gPlayer->pos);
            restCount++;
        }
    }

    // 少于2个马上生成一个新的, 如果没到10个则每10秒生成一个新的
    if (restCount < 2 || lastGenTime + 10.0f < gameTime && restCount < 10)
    {
        lastGenTime = gameTime;
        GenerateEnemy();
    }

    checkCount = collisionChecker.checkCount;
    collisionChecker.checkCount = 0;
    PROFILE_CHECKPOINT(EntityManagerStep);
    EntityManager::getSingleton().step(gameTime, deltaTime);
    PROFILE_ENDPOINT();
}

void MainGameState::OnRender()
{
    PROFILE_CHECKPOINT(Render);
    this->hge->Gfx_BeginScene(0);
    this->hge->Gfx_Clear(0);//黑色背景
    float gameTime = this->hge->Timer_GetTime();
    float deltaTime = this->hge->Timer_GetDelta();
    this->hge->Gfx_SetTransform();
    this->gui->Render();
    EntityManager::getSingleton().render(gameTime, deltaTime);
    RenderQueue::getSingleton().flush();
    static char buf[256];
    sprintf(buf, "entity: %d", EntityManager::getSingleton().getCount());
    font->Render(100, 70, TA_LEFT, buf);
    sprintf(buf, "cpf: %d", checkCount);
    font->Render(100, 100, TA_LEFT, buf);
    sprintf(buf, "fps: %d", hge->Timer_GetFPS());
    font->Render(100, 130, TA_LEFT, buf);


    PROFILE_CHECKPOINT(ProfileRender);
    const vector<Profile::SumNode> &sn = Profile::getSingleton().getSummary();
    float x = 0.f;
    float y = 0.f;
    sprintf(buf, "totalTime(s) max(ms) logCount averTime(ms) lastTime(ms)");
    font->Render(x, y, TA_LEFT, "name");
    font->Render(x + 100.f, y, TA_LEFT, buf);
    y += 20.f;
    for (vector<Profile::SumNode>::const_iterator isn = sn.begin(); isn != sn.end(); ++isn)
    {
        struct SumNode
        {
            string name;
            float totalTime;
            float maxTime;
            size_t logCount;
        };
        sprintf(buf, "%10.2f%10.2f%10d%10.2f%10.2f", isn->totalTime, isn->maxTime * 1000, isn->logCount, isn->totalTime * 1000 / isn->logCount, isn->lastTime * 1000);
        font->Render(x, y, TA_LEFT, isn->name.c_str());
        font->Render(x + 100.f, y, TA_LEFT, buf);
        y += 20.f;
    }
    // 游戏状态信息
    this->hge->Gfx_EndScene();
    PROFILE_ENDPOINT();
}

Enemy *MainGameState::getNearestEnemy(const Point2f &pos)
{
    Enemy *pEnemy = 0;
    float minLen = 1e10;
    for (list<AiEnemyController*>::iterator ie = enemies.begin(); ie != enemies.end(); ++ie)
    {
        Enemy &enemy = *(*ie)->enemy;
        float len = (enemy.pos - pos).Length();
        if (pEnemy == 0 || len < minLen)
        {
            pEnemy = &enemy;
            minLen = len;
        }
    }
    return pEnemy;
}