#include "hge.h"
#include "game/maingamestate.h"
#include "menu/mainmenustate.h"
#include "common/entitymanager.h"
#include "common/profile.h"

int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{	
    //ÉèÖÃHGEÊôĞÔ		
    HGE *hge = hgeCreate(HGE_VERSION);
    hge->System_SetState(HGE_LOGFILE, "SysInfo.log");
    hge->System_SetState(HGE_INIFILE, "SysINI.ini");
    hge->System_SetState(HGE_TITLE, "TankWar");
    hge->System_SetState(HGE_WINDOWED, true);
    hge->System_SetState(HGE_SCREENWIDTH, 800);
    hge->System_SetState(HGE_SCREENHEIGHT, 600);
    hge->System_SetState(HGE_SCREENBPP, 32);
    hge->System_SetState(HGE_FPS, 60);
    hge->System_SetState(HGE_DONTSUSPEND, true);
    hge->System_SetState(HGE_HIDEMOUSE, false);

#ifdef _DEBUG
#if HGE_VERSION >= 0x170
    hge->System_SetState(HGE_SHOWSPLASH, false);
#else
    hge->System_SetState(hgeIntState(14), 0xFACE0FF);
#endif
#endif
    GameStateManager gsm;
    EntityManager em;
    Profile pf;
    {
        MainMenuState mms;
        mms.SetName("mainmenu");
        GameStateManager::getSingleton().RegisterState(&mms);

        MainGameState mgs;
        mgs.SetName("maingame");
        GameStateManager::getSingleton().RegisterState(&mgs);

        GameStateManager::getSingleton().RequestState("maingame");
        if (hge->System_Initiate())
        {
            hge->System_Start();
        }
        GameStateManager::getSingleton().releaseSingleton();
    }
    hge->System_Shutdown();
    hge->Release();
    return 0;
}
