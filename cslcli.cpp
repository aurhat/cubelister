#include "Csl.h"
#include "CslTools.h"
#include "CslEngine.h"
#include "CslGameSauerbraten.h"

#if !wxCHECK_VERSION(2,9,0)
#error wxWidgets >= 2.9.0 is required!
#endif //!wxCHECK_VERSION(2,9,0)

#define CSL_TIMER_SHOT 250

class CslApp: public wxAppConsole
{
    private:
        CslEngine *m_engine;
        wxTimer m_timer;
        virtual bool OnInit();
        virtual int OnRun();
        virtual int OnExit();

        void OnTimer(wxTimerEvent& event);
        void OnResolve(CslResolveEvent& event);
        void OnPong(CslPongEvent& event);

        DECLARE_EVENT_TABLE()
};

DECLARE_APP(CslApp);
IMPLEMENT_APP(CslApp)

BEGIN_EVENT_TABLE(CslApp,wxAppConsole)
    EVT_TIMER(wxID_ANY,CslApp::OnTimer)
    CSL_EVT_RESOLVE(CslApp::OnResolve)
    CSL_EVT_PONG(wxID_ANY,CslApp::OnPong)
END_EVENT_TABLE()

bool CslApp::OnInit()
{
    if (argc<3)
    {
        if (argc==2 && argv[1]==wxT("-v"))
        {
            wxPrintf("using %s engine: %s\ncompiled using: %s\n",
                     CSL_NAME_STR,
                     CSL_VERSION_LONG_STR,
                     CSL_WXVERSION_STR);
        }
        else
            wxPrintf("%s: Usage: <Host> <Port>\n",argv[0]);
        return false;
    }

    m_engine=new CslEngine;

    if (!m_engine->Init(this,5000,1000/CSL_TIMER_SHOT))
        return false;

    CslGame *game=new CslGameSauerbraten();

    if (!m_engine->AddGame(game))
        return false;

    long port;
    wxString(argv[2]).ToLong(&port);

    CslServerInfo *info=new CslServerInfo(game,argv[1],port%(1<<16));
    game->AddServer(info);

    info->PingExt(true);

    if (info->Pingable)
        m_engine->Ping(info,true);    
    else
        m_engine->ResolveHost(info);

    return true;
}

int CslApp::OnRun()
{
    wxEventLoop loop;
    m_timer.SetOwner(this);
    m_timer.Start(CSL_TIMER_SHOT);

    return loop.Run();
}

int CslApp::OnExit()
{
    if (m_timer.IsRunning())
        m_timer.Stop();
    if (m_engine)
    {
        m_engine->DeInit();
        delete m_engine;
    }

    return 0;
}

void CslApp::OnTimer(wxTimerEvent& event)
{
    static wxInt32 count=0;

    if (++count>=5000/CSL_TIMER_SHOT)
        wxEventLoop::GetActive()->Exit(1);
    else
        m_engine->CheckResends();
}

void CslApp::OnResolve(CslResolveEvent& event)
{
    CslServerInfo *info=event.GetServerInfo();

    if (info->Pingable)
        m_engine->Ping(info,true);
    else
        wxEventLoop::GetActive()->Exit(1);
}

void CslApp::OnPong(CslPongEvent& event)
{
    CslServerInfo *info=event.GetServerInfo();

    switch (event.GetId())
    {
        case CslPongEvent::PONG:
            wxPrintf(wxT("Host:%s  Ping:%dms  Mode:%s  Map:%s  "
                         "Time:%d  Players:%d/%d  MM:%s\n"),
                     info->GetBestDescription(),
                     info->Ping,info->GameMode,
                     info->Map,info->TimeRemain,
                     info->Players,info->PlayersMax,
                     info->MMDescription);
            break;

        case CslPongEvent::UPTIME:
            m_engine->PingExPlayerInfo(info,-1,true);
            break;

        case CslPongEvent::PLAYERSTATS:
            loopv(info->PlayerStats.m_stats)
            {
                CslPlayerStatsData *data=info->PlayerStats.m_stats[i];

                wxPrintf("%2d: Name:%-15.15s(%3d / %15.15s)  F:%3d  "
                         "D:%3d  Tk:%3d  Acc:%3d%%\n",i+1,
                         data->Name,data->ID,Int2IP(data->IP),
                         data->Frags,data->Deaths,
                         data->Teamkills,data->Accuracy);
            }
            wxEventLoop::GetActive()->Exit(0);
            break;
    }
}
