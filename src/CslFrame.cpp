/***************************************************************************
 *   Copyright (C) 2007 by Glen Masgai                                     *
 *   mimosius@gmx.de                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <wx/mstream.h>
#include <wx/fileconf.h>
#include <wx/sysopt.h>
#include <wx/protocol/http.h>
#include <wx/process.h>
#include "engine/CslTools.h"
#include "engine/CslVersion.h"
#include "CslFrame.h"
#include "CslDlgAbout.h"
#include "CslDlgAddMaster.h"
#include "CslDlgConnectWait.h"
#include "CslDlgGeneric.h"
#include "CslConnectionState.h"
#include "CslGeoIP.h"
#include "engine/CslGameSauerbraten.h"
#include "engine/CslGameAssaultCube.h"
#include "engine/CslGameBloodFrontier.h"
#include "engine/CslGameCube.h"

#ifndef __WXMSW__
#include "csl_icon_png.h"
#endif
#include "img/master_24.xpm"
#include "img/close_14.xpm"
#include "img/close_high_14.xpm"
#include "img/close_press_14.xpm"

#define CSL_TIMER_SHOT 250

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK,wxID_ANY)
DECLARE_EVENT_TYPE(wxCSL_EVT_PROCESS,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_VERSIONCHECK(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_VERSIONCHECK,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),


#define CSL_EVT_PROCESS(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PROCESS,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

DEFINE_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK)
DEFINE_EVENT_TYPE(wxCSL_EVT_PROCESS)


enum
{
    CSL_BUTTON_SEARCH_CLOSE = MENU_END + 1,
    CSL_TEXT_SEARCH,
    CSL_BUTTON_SEARCH,
    CSL_RADIO_SEARCH_SERVER,
    CSL_RADIO_SEARCH_PLAYER
};


BEGIN_EVENT_TABLE(CslFrame, wxFrame)
    CSL_EVT_PONG(wxID_ANY,CslFrame::OnPong)
    EVT_TIMER(wxID_ANY,CslFrame::OnTimer)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslFrame::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslFrame::OnListItemActivated)
    EVT_TREE_SEL_CHANGED(wxID_ANY,CslFrame::OnTreeSelChanged)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,CslFrame::OnTreeRightClick)
    EVT_MENU(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_TEXT(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_RADIOBUTTON(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslFrame::OnCommandEvent)
    // dont use wxID_ANY for EVT_BUTTON, because on wxMAC
    // wxID_CANCEL is sent when pressing ESC
    EVT_BUTTON(CSL_BUTTON_SEARCH,CslFrame::OnCommandEvent)
    EVT_BUTTON(CSL_BUTTON_SEARCH_CLOSE,CslFrame::OnCommandEvent)
    // on wxMSW neither EVT_CHAR nor EVT_KEY_xxx
    // events are send, so connect them in the ctor()
    #ifdef __WXMAC__
    EVT_CHAR(CslFrame::OnKeypress)
    #else
    // on wxGTK EVT_CHAR gets not send since version >= 2.8.5
    EVT_KEY_UP(CslFrame::OnKeypress)
    #endif
    EVT_SHOW(CslFrame::OnShow)
    EVT_CLOSE(CslFrame::OnClose)
    CSL_EVT_VERSIONCHECK(wxID_ANY,CslFrame::OnVersionCheck)
    CSL_EVT_PROCESS(wxID_ANY,CslFrame::OnEndProcess)
END_EVENT_TABLE()


class CslIDMapping
{
    public:
        CslIDMapping(wxInt32 oldId,wxInt32 newId) :
                m_oldId(oldId), m_newId(newId) {}

        wxInt32 m_oldId,m_newId;
};


class CslTreeItemData : public wxTreeItemData
{
    public:
        CslTreeItemData(CslGame* game) : m_game(game), m_master(NULL) {}
        CslTreeItemData(CslMaster* master) : m_game(NULL), m_master(master) {}

        CslGame* GetGame() { return m_game; }
        CslMaster* GetMaster() { return m_master; }

    protected:
        CslGame *m_game;
        CslMaster *m_master;
};

class CslProcess : public wxProcess
{
    public:
        CslProcess(wxWindow *parent,CslServerInfo *info,const wxString& cmd) :
                wxProcess((CslListCtrlServer*)parent),
                m_parent(parent),m_info(info),m_cmd(cmd)
        {
            m_self=this;
            m_watch.Start(0);
            Redirect();
        }

        virtual void OnTerminate(int pid,int code)
        {
            m_watch.Pause();

            wxUint32 time=m_watch.Time()/1000;
            if (time>g_cslSettings->minPlaytime)
                m_info->SetLastPlayTime(time);

            // Cube returns with 1 - weird
            if (!m_info->GetGame().ReturnOk(code))
                wxMessageBox(wxString::Format(_("%s returned with code: %d"),
                                              m_cmd.c_str(),code),_("Error"),wxICON_ERROR,m_parent);

            ProcessInputStream();
            ProcessErrorStream();

            if (g_cslSettings->autoSaveOutput && !g_cslSettings->gameOutputPath.IsEmpty())
                CslDlgOutput::SaveFile(g_cslSettings->gameOutputPath);

            wxCommandEvent evt(wxCSL_EVT_PROCESS);
            evt.SetClientData(m_info);
            wxPostEvent(m_parent,evt);

            m_self=NULL;

            delete this;
        }

        static void ProcessInputStream()
        {
            if (!m_self)
                return;

            wxInputStream *stream=m_self->GetInputStream();
            if (!stream)
                return;

            while (stream->CanRead())
            {
                char buf[1025];
                stream->Read((void*)buf,1024);
                wxUint32 last=stream->LastRead();
                if (last==0)
                    break;
                buf[last]=0;
                //Cube has color codes in it's outputm
                m_self->m_info->GetGame().ProcessOutput(buf,&last);
                CslDlgOutput::AddOutput(buf,last);
                //LOG_DEBUG("%s", buf);
            }
        }

        static void ProcessErrorStream()
        {
            if (!m_self)
                return;

            wxInputStream *stream=m_self->GetErrorStream();
            if (!stream)
                return;

            while (stream->CanRead())
            {
                char buf[1025];
                stream->Read((void*)buf,1024);

                wxUint32 last=stream->LastRead();
                if (last==0)
                    break;
                buf[last]=0;
                /*
                //Cube has color codes in it's outputm
                m_self->m_info->GetGame().ProcessOutput(buf,&last);
                CslDlgOutput::AddOutput(buf,last);
                */
                LOG_DEBUG("%s",buf);
            }
        }

    protected:
        static CslProcess *m_self;
        wxWindow *m_parent;
        CslServerInfo *m_info;
        wxString m_cmd;
        bool m_clear;
        wxStopWatch m_watch;
};

CslProcess* CslProcess::m_self=NULL;


wxThread::ExitCode CslVersionCheckThread::Entry()
{
    wxString *version=NULL;

    wxHTTP http;
    http.SetTimeout(10);

    if (http.Connect(CSL_WEBADDR_STR,80))
    {
        http.SetHeader(wxT("User-Agent"),GetHttpAgent());
        wxInputStream *data=http.GetInputStream(wxT("/latest.txt"));
        wxUint32 code=http.GetResponse();

        if (data)
        {
            if (code==200)
            {
                size_t size=min(data->GetSize(),15);
                if (size)
                {
                    char buf[16]={0};
                    data->Read((void*)buf,size);

                    if (!(strstr(buf,"<html>") ||
                          strstr(buf,"<HTML>")))
                    {
                        char *pend=strpbrk(buf," \t\r\n");
                        if (pend)
                            *pend='\0';
                        version=new wxString(A2U(buf));
                        LOG_DEBUG("version check: %s\n",buf);
                    }
                }
            }
            delete data;
        }
    }

    wxCommandEvent evt(wxCSL_EVT_VERSIONCHECK);
    evt.SetClientData(version);
    wxPostEvent(m_evtHandler,evt);

    LOG_DEBUG("version check: thread exit\n");

    return 0;
}


CslFrame::CslFrame(wxWindow* parent,int id,const wxString& title,
                   const wxPoint& pos,const wxSize& size,long style):
        wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
    m_oldSelectedInfo=NULL;

    m_engine=new CslEngine(this);
    if (m_engine->Init())
    {
        m_engine->AddGame(new CslGameSauerbraten());
        m_engine->AddGame(new CslGameAssaultCube());
        m_engine->AddGame(new CslBloodFrontier());
        m_engine->AddGame(new CslGameCube());
    }

    LoadSettings();

    m_engine->SetUpdateInterval(g_cslSettings->updateInterval);

    m_imgListTree.Create(24,24,true);
    m_imgListTree.Add(wxBitmap(master_24_xpm));

    m_imgListButton.Create(14,14,true);
    m_imgListButton.Add(wxBitmap(close_14_xpm));
    m_imgListButton.Add(wxBitmap(close_high_14_xpm));
    m_imgListButton.Add(wxBitmap(close_press_14_xpm));

    CslListCtrlPlayer::CreateImageList();

    m_outputDlg=new CslDlgOutput(this);
    m_extendedDlg=new CslDlgExtended(this);
    m_trafficDlg=NULL;

    CreateControls();
    SetProperties();
    bool serverLoaded=LoadServers();
    DoLayout();

    pane_main->Connect(wxEVT_AUI_PANE_CLOSE,wxAuiManagerEventHandler(CslFrame::OnPaneClose),NULL,this);

#ifdef __WXMSW__  // key events from childs get send to its parent
    list_ctrl_master->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    list_ctrl_favourites->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    list_ctrl_info->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    tree_ctrl_games->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    text_ctrl_search->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
#endif

    tree_ctrl_games->SetImageList(&m_imgListTree);
    m_treeGamesRoot=tree_ctrl_games->AddRoot(wxEmptyString,-1,-1);

    if (m_engine->IsOk())
    {
        CslGame *game;
        vector<CslGame*>& games=m_engine->GetGames();

        loopv(games)
        {
            game=games[i];

            if (!serverLoaded || (serverLoaded && game->GetMasters().empty()))
            {
                game->AddMaster(new CslMaster(game->GetDefaultMasterConnection()));
                //add default servers here
            }

            const char **icon=game->GetIcon(24);
            bool select=g_cslSettings->lastGame.IsEmpty() ? i==0 :
                        g_cslSettings->lastGame==game->GetName();
            m_imgListTree.Add(icon ? wxBitmap(icon):wxBitmap(24,24));
            TreeAddGame(game,icon ? i+1:-1,select);
        }

        CslMenu::EnableMenuItem(MENU_MASTER_ADD);

        //tree_ctrl_games->ExpandAll();

        m_timerUpdate=g_cslSettings->updateInterval/CSL_TIMER_SHOT;
        m_timerCount=0;
        m_timerInit=true;
        m_timer.SetOwner(this);
        m_timer.Start(CSL_TIMER_SHOT);
    }
    else
    {
        wxMessageBox(_("Failed to initialise internal engine!\n"\
                       "Please try to restart the application."),
                     _("Fatal error!"),wxICON_ERROR,this);
        m_engine->DeInit();
        delete m_engine;
        m_engine=NULL;
    }

    m_versionCheckThread=new CslVersionCheckThread(this);
    m_versionCheckThread->Run();
}

CslFrame::~CslFrame()
{
    if (m_timer.IsRunning())
        m_timer.Stop();

    if (CslConnectionState::IsPlaying())
        CslConnectionState::GetInfo()->GetGame().GameEnd();

    if (m_engine)
    {
        SaveSettings();
        SaveServers();
        m_engine->DeInit();
        delete m_engine;
    }

    delete g_cslSettings;

    if (m_versionCheckThread)
    {
        m_versionCheckThread->Delete();
        delete m_versionCheckThread;
    }

    delete m_menu;

    m_AuiMgr.UnInit();
}

void CslFrame::CreateMainMenu()
{
    wxMenu *menu;

    menubar=new wxMenuBar();
    // Do not add the File menu on __wxMAC__, since Preferences and Exit getting moved
    // to the "Mac menu" so the File menu is empty then.
    // Add Prefernces and Exit to any other menu.
#ifndef __WXMAC__
    menu=new wxMenu();
    menubar->Append(menu,_("&File"));
    CslMenu::AddItem(menu,wxID_PREFERENCES,_("&Settings"),wxART_SETTINGS);
    CslMenu::AddItem(menu,wxID_EXIT,_("&Exit"),wxART_QUIT);
#endif

    menuMaster=new wxMenu();
    CslMenu::AddItem(menuMaster,MENU_MASTER_UPDATE,_("&Update from master\tF5"),wxART_RELOAD);
    menuMaster->AppendSeparator();
    CslMenu::AddItem(menuMaster,MENU_MASTER_ADD,_("Add a &new master ..."),wxART_ADD_BOOKMARK);
    CslMenu::AddItem(menuMaster,MENU_MASTER_DEL,_("&Remove master"),wxART_DEL_BOOKMARK);
    menubar->Append(menuMaster,_("&Master"));
#ifdef __WXMAC__
    CslMenu::AddItem(menuMaster,wxID_PREFERENCES,_("&Settings"),wxART_SETTINGS);
    CslMenu::AddItem(menuMaster,wxID_EXIT,_("&Exit"),wxART_QUIT);
    CslMenu::AddItem(menuMaster,wxID_ABOUT,_("A&bout"),wxART_ABOUT);
#endif

    menu=new wxMenu();
    CslMenu::AddItem(menu,MENU_VIEW_GAMES,_("Show games"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_SERVER_INFO,_("Show server information"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_PLAYER_LIST,_("Show player list"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_FAVOURITES,_("Show favourites"),
                     wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(menu,MENU_VIEW_SEARCH,_("Show search bar\tCTRL+F"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_OUTPUT,_("Show game output\tCTRL+O"),
                     wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(menu,MENU_VIEW_AUTO_SORT,_("Sort lists automatically\tCTRL+L"),
                     wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(menu,MENU_VIEW_TRAFFIC,_("&Traffic statistics"),wxART_NONE);
    menubar->Append(menu,_("&View"));

#ifndef __WXMAC__
    menu=new wxMenu();
    CslMenu::AddItem(menu,wxID_ABOUT,_("A&bout"),wxART_ABOUT);
    menubar->Append(menu,_("&Help"));
#endif

    SetMenuBar(menubar);
    m_menu=new CslMenu(menubar);
}

void CslFrame::CreateControls()
{
    long listStyle=wxLC_REPORT;
    long treeStyle=wxTR_NO_LINES|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT|wxTR_DEFAULT_STYLE;

#ifdef __WXMSW__
    listStyle|=wxNO_BORDER;
    listStyle|=wxNO_BORDER;
#else
    listStyle|=wxSUNKEN_BORDER;
    treeStyle|=wxSUNKEN_BORDER;
#endif // __WXMSW__

    pane_main=new wxPanel(this,wxID_ANY);
    tree_ctrl_games=new wxTreeCtrl(pane_main,wxID_ANY,wxDefaultPosition,wxDefaultSize,treeStyle);
    list_ctrl_info=new CslListCtrlInfo(pane_main,wxID_ANY,wxDefaultPosition,wxDefaultSize,listStyle|wxLC_NO_HEADER);
    list_ctrl_master=new CslListCtrlServer(pane_main,CslListCtrlServer::CSL_LIST_MASTER,
                                           wxDefaultPosition,wxDefaultSize,listStyle);
    list_ctrl_favourites=new CslListCtrlServer(pane_main,CslListCtrlServer::CSL_LIST_FAVOURITE,
            wxDefaultPosition,wxDefaultSize,listStyle);
    pane_search=new wxPanel(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxNO_BORDER|wxTAB_TRAVERSAL);
    text_ctrl_search=new wxTextCtrl(pane_search,CSL_TEXT_SEARCH,wxEmptyString,
                                    wxDefaultPosition,wxDefaultSize,wxTE_RICH|wxTE_RICH2|wxTE_PROCESS_ENTER);
    text_search_result=new wxStaticText(pane_search,wxID_ANY,wxEmptyString);
    button_search=new wxButton(pane_search,CSL_BUTTON_SEARCH,_("&Search"));
    button_search_clear=new wxBitmapButton(pane_search,CSL_BUTTON_SEARCH_CLOSE,wxNullBitmap,
                                           wxDefaultPosition,wxDefaultSize,wxNO_BORDER);
    gauge_search=new wxGauge(pane_search,wxID_ANY,0,wxDefaultPosition,wxSize(button_search->GetBestSize().x,16));
    radio_search_server=new wxRadioButton(pane_search,CSL_RADIO_SEARCH_SERVER,_("Se&rvers"),
                                          wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
    radio_search_player=new wxRadioButton(pane_search,CSL_RADIO_SEARCH_PLAYER,_("&Players"));
    list_ctrl_players=new CslListCtrlPlayer(pane_main,wxID_ANY,wxDefaultPosition,wxDefaultSize,listStyle);
    m_playerLists.add(list_ctrl_players);

    CreateMainMenu();

    CslStatusBar *statusBar=new CslStatusBar(this);
    SetStatusBar(statusBar);
    CslStatusBar::InitBar(statusBar);
}

void CslFrame::SetProperties()
{
#ifndef __WXMSW__
    m_AuiMgr.SetFlags(wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_HINT_FADE |
                      wxAUI_MGR_VENETIAN_BLINDS_HINT | wxAUI_MGR_NO_VENETIAN_BLINDS_FADE);
#endif
    m_AuiMgr.SetManagedWindow(pane_main);

    SetTitle(_("Cube Server Lister"));

    gauge_search->Enable(false);
    gauge_search->Hide();
    button_search->Enable(false);
    button_search->Hide();
    text_search_result->SetLabel(wxString::Format(_("Search result: %d servers"),0));
    button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(0));
    button_search_clear->SetSize(button_search_clear->GetBestSize());

    button_search_clear->Connect(wxEVT_ENTER_WINDOW,
                                 wxMouseEventHandler(CslFrame::OnMouseEnter),NULL,this);
    button_search_clear->Connect(wxEVT_LEAVE_WINDOW,
                                 wxMouseEventHandler(CslFrame::OnMouseLeave),NULL,this);
    button_search_clear->Connect(wxEVT_LEFT_DOWN,
                                 wxMouseEventHandler(CslFrame::OnMouseLeftDown),NULL,this);

    list_ctrl_players->ListInit(CslListCtrlPlayer::CSL_LISTPLAYER_MINI_SIZE);

    list_ctrl_master->ListInit(m_engine,list_ctrl_favourites);
    list_ctrl_favourites->ListInit(m_engine,list_ctrl_master);

    // see wx_wxbitmap.html
#ifdef __WXMSW__
    SetIcon(wxICON(aa_csl_48));
#else
    wxMemoryInputStream stream(csl_icon_png,sizeof(csl_icon_png));
    wxIcon icon;
    wxImage image(stream,wxBITMAP_TYPE_PNG);
    wxBitmap bitmap(image);
    icon.CopyFromBitmap(bitmap);
    SetIcon(icon);
#endif

    SetMinSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT));
}

void CslFrame::DoLayout()
{
    wxInt32 i=0;

    sizer_main = new wxFlexGridSizer(2,1,0,0);
    sizer_main->Add(pane_main,1,wxEXPAND,0);
    if (g_cslSettings->showSearch)
        sizer_main->Add(pane_search,0,wxEXPAND);
    else
        pane_search->Hide();
    sizer_main->AddGrowableRow(0);
    sizer_main->AddGrowableCol(0);
    SetSizer(sizer_main);

    sizer_search=new wxFlexGridSizer(1,9,0,0);
    wxStaticText* label_search_static=new wxStaticText(pane_search,wxID_ANY,_("Search:"));
#ifdef __WXMSW__
    i=2; //bitmap button offset
#endif
    sizer_search->AddSpacer(4);
    sizer_search->Add(button_search_clear,0,wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL,i);
    sizer_search->Add(label_search_static,0,wxLEFT|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL,8);
    sizer_search->Add(text_ctrl_search,0,wxLEFT|wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    sizer_search->Add(text_search_result,0,wxLEFT|wxRIGHT|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL,4);
    sizer_search->Add(button_search,0,wxLEFT|wxRIGHT|wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_VERTICAL,4);
    sizer_search->Add(gauge_search,0,wxLEFT|wxRIGHT|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL,4);
    sizer_search->Add(radio_search_server,0,wxLEFT|wxRIGHT|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL,4);
    sizer_search->Add(radio_search_player,0,wxLEFT|wxRIGHT|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL,4);
    pane_search->SetSizer(sizer_search);
    sizer_search->AddGrowableCol(3);
    //sizer_search->AddGrowableCol(3);

#define CSL_CAPTION_MASTER_LIST_SERVERS _("Master list servers")
#define CSL_CAPTION_FAVOURITE_LIST_SERVERS _("Favourite servers")
#define CSL_CAPTION_PLAYERS_SELECTED _("Selected server")
#define CSL_CAPTION_SERVER_INFO _("Server info")

    wxSize size=list_ctrl_master->GetBestSize();

    m_AuiMgr.AddPane(list_ctrl_master,wxAuiPaneInfo().Name(wxT("masterlist")).
                     CloseButton(false).Center().BestSize(size).MinSize(size.x,20));
    m_AuiMgr.AddPane(list_ctrl_favourites,wxAuiPaneInfo().Name(wxT("favlist")).
                     Bottom().Row(2).BestSize(size).MinSize(size).FloatingSize(600,240));
    m_AuiMgr.AddPane(tree_ctrl_games,wxAuiPaneInfo().Name(wxT("games")).
                     Caption(_("Games")).Left().Layer(1).Position(0).
                     MinSize(240,20).BestSize(240,250).FloatingSize(240,250));
    m_AuiMgr.AddPane(list_ctrl_info,wxAuiPaneInfo().Name(wxT("info")).
                     Left().Layer(1).Position(1).
                     MinSize(240,20).BestSize(360,200).FloatingSize(360,200));
    m_AuiMgr.AddPane(list_ctrl_players,wxAuiPaneInfo().Name(wxT("players0")).
                     Left().Layer(1).Position(2).
                     MinSize(240,20).BestSize(CslListCtrlPlayer::BestSizeMini).FloatingSize(280,200));
    if (!g_cslSettings->layout.IsEmpty())
        m_AuiMgr.LoadPerspective(g_cslSettings->layout,false);

    wxAuiPaneInfo *pane;

    pane=&m_AuiMgr.GetPane(tree_ctrl_games);
    if (pane->IsOk())
        CslMenu::CheckMenuItem(MENU_VIEW_GAMES,pane->IsShown());
    pane=&m_AuiMgr.GetPane(list_ctrl_info);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_SERVER_INFO);
        CslMenu::CheckMenuItem(MENU_VIEW_SERVER_INFO,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(list_ctrl_master);
    if (pane->IsOk())
        pane->Caption(CSL_CAPTION_MASTER_LIST_SERVERS);
    pane=&m_AuiMgr.GetPane(list_ctrl_favourites);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_FAVOURITE_LIST_SERVERS);
        CslMenu::CheckMenuItem(MENU_VIEW_FAVOURITES,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(list_ctrl_players);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYERS_SELECTED);
        CslMenu::CheckMenuItem(MENU_VIEW_PLAYER_LIST,pane->IsShown());
    }

    SetListCaption(CslListCtrlServer::CSL_LIST_MASTER);
    SetListCaption(CslListCtrlServer::CSL_LIST_FAVOURITE);

    m_AuiMgr.Update();

    sizer_main->Fit(this);
    Layout();
    SetSize(g_cslSettings->frameSize);

    CentreOnScreen();
}

wxString CslFrame::PlayerListGetCaption(CslServerInfo *info,bool selected)
{
    wxString caption;

    if (selected)
    {
        caption+=CSL_CAPTION_PLAYERS_SELECTED;
        if (info)
            caption+=wxT(": ");
    }

    if (info)
        caption+=info->GetBestDescription()+wxT(" (")+info->GetGame().GetName()+wxT(")");

    return caption;
}

void CslFrame::PlayerListCreateView(CslServerInfo *info,wxUint32 view,const wxString& name)
{
    long style=wxLC_REPORT;

#ifdef __WXMSW__
    style|=wxNO_BORDER;
#else
    style|=wxSUNKEN_BORDER;
#endif // __WXMSW__

    wxSize size=view==CslListCtrlPlayer::CSL_LISTPLAYER_MICRO_SIZE ?
                CslListCtrlPlayer::BestSizeMicro:CslListCtrlPlayer::BestSizeMini;

    wxAuiPaneInfo pane;

    pane.BestSize(size).DestroyOnClose().Caption(PlayerListGetCaption(info,false));

    if (name.IsEmpty())
    {
        long l;
        wxInt32 j,id=1;
        vector<wxInt32> ids;

        loopv(m_playerLists)
        {
            if (i==0)
                continue;

            wxAuiPaneInfo &pane=m_AuiMgr.GetPane(m_playerLists[i]);
            if (pane.IsOk() && pane.name.Mid(7).ToLong(&l))
            {
                for (j=0;j<ids.length();j++)
                {
                    if (l<ids[j])
                        break;
                }
                ids.insert(j,l);
            }
        }

        loopv(ids)
        {
            if (id<ids[i])
                break;
            else
                id++;
        }

        pane.Float().FloatingPosition(::wxGetMousePosition()).
        Name(wxString::Format(wxT("players%d"),id));
    }
    else
        pane.Name(name);

    pane_main->Freeze();

    CslListCtrlPlayer *list=new CslListCtrlPlayer(pane_main,wxID_ANY,wxDefaultPosition,wxDefaultSize,style);
    list->ListInit(view);
    list->SetInfo(info);

    m_AuiMgr.AddPane(list,pane);
    m_AuiMgr.Update();

    pane_main->Thaw();

    m_playerLists.add(list);

    info->SetPingExt(true);

    m_engine->PingEx(info,true);
}

void CslFrame::TogglePane(wxInt32 id)
{
    wxAuiPaneInfo *pane;

    switch (id)
    {
        case MENU_VIEW_GAMES:
            pane=&m_AuiMgr.GetPane(tree_ctrl_games);
            break;
        case MENU_VIEW_SERVER_INFO:
            pane=&m_AuiMgr.GetPane(list_ctrl_info);
            break;
        case MENU_VIEW_PLAYER_LIST:
            pane=&m_AuiMgr.GetPane(list_ctrl_players);
            break;
        case MENU_VIEW_FAVOURITES:
            pane=&m_AuiMgr.GetPane(list_ctrl_favourites);
            break;
        default:
            return;
    }

    if (!pane->IsOk())
        return;

    bool shown=pane->IsShown();

    pane->Show(!shown);
    CslMenu::CheckMenuItem(id,!shown);

    m_AuiMgr.Update();
}

void CslFrame::ToggleSearchBar()
{
    if (g_cslSettings->showSearch && !pane_search->IsShown())
    {
        sizer_main->Add(pane_search,0,wxEXPAND);
        pane_search->Show();
        text_ctrl_search->SetFocus();
    }
    else if (!g_cslSettings->showSearch && pane_search->IsShown())
    {
        pane_search->Hide();
        text_ctrl_search->Clear();
        text_ctrl_search->SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
        list_ctrl_master->ListSearch(wxEmptyString);
        sizer_main->Detach(pane_search);
    }
    else
        return;

    sizer_main->Layout();
}

void CslFrame::SetTotalPlaytime(CslGame* game)
{
    if (!game)
        return;

    wxString s;
    wxUint32 playtime=0;
    CslServerInfo *info=NULL;
    vector<CslServerInfo*>& servers=game->GetServers();

    loopv(servers)
    {
        if (info)
        {
            if (info->PlayTimeTotal<servers[i]->PlayTimeTotal)
                info=servers[i];
        }
        else
            info=servers[i];

        playtime+=servers[i]->PlayTimeTotal;
    }

    if (playtime)
    {
        s=wxString::Format(_("Total play time for game %s: %s"),
                           game->GetName().c_str(),
                           FormatSeconds(playtime).c_str());
        if (info)
        {
            s+=wxT("  -  ");
            s+=wxString::Format(_("most played server: \'%s\' (total play time: %s)"),
                                info->GetBestDescription().c_str(),
                                FormatSeconds(info->PlayTimeTotal).c_str());
        }
    }

    CslStatusBar::SetText(1,s);
}

void CslFrame::SetListCaption(wxInt32 id,const wxString& addon)
{
    wxWindow *panel;
    wxUint32 filter;
    wxString caption;

    if (id==CslListCtrlServer::CSL_LIST_MASTER)
    {
        panel=list_ctrl_master;
        filter=g_cslSettings->filterMaster;
        caption=CSL_CAPTION_MASTER_LIST_SERVERS;
    }
    else if (id==CslListCtrlServer::CSL_LIST_FAVOURITE)
    {
        panel=list_ctrl_favourites;
        filter=g_cslSettings->filterFavourites;
        caption=CSL_CAPTION_FAVOURITE_LIST_SERVERS;
    }
    else
        return;

    wxAuiPaneInfo& pane=m_AuiMgr.GetPane(panel);
    if (!pane.IsOk())
        return;

    if (filter)
    {
        caption+=_("  (Filters: ");
        if (filter&CSL_FILTER_OFFLINE)
            caption+=MENU_SERVER_FILTER_OFF_STR+wxString(wxT(", "));
        if (filter&CSL_FILTER_FULL)
            caption+=MENU_SERVER_FILTER_FULL_STR+wxString(wxT(", "));
        if (filter&CSL_FILTER_EMPTY)
            caption+=MENU_SERVER_FILTER_EMPTY_STR+wxString(wxT(", "));
        if (filter&CSL_FILTER_NONEMPTY)
            caption+=MENU_SERVER_FILTER_NONEMPTY_STR+wxString(wxT(", "));
        if (filter&CSL_FILTER_MM2)
            caption+=MENU_SERVER_FILTER_MM2_STR+wxString(wxT(", "));
        if (filter&CSL_FILTER_MM3)
            caption+=MENU_SERVER_FILTER_MM3_STR+wxString(wxT(", "));
        caption.Remove(caption.Length()-2);
        caption+=wxT(")");
    }

    caption+=wxT(" ")+addon;

    pane.Caption(caption);
    m_AuiMgr.Update();
}

void CslFrame::SetSearchbarColour(bool value)
{
    if (!value)
    {
#ifdef __WXMAC__
        // very ugly - setting back to black doesnt work, so add 1
        text_ctrl_search->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT).Red()+1);
#else
        text_ctrl_search->SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
        text_ctrl_search->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
#endif
    }
    else
    {
#ifdef __WXMAC__
        text_ctrl_search->SetForegroundColour(*wxRED);
#else
        text_ctrl_search->SetBackgroundColour(wxColour(255,100,100));
        text_ctrl_search->SetForegroundColour(*wxWHITE);
#endif
    }

#ifdef __WXMSW__
    text_ctrl_search->Refresh();
#endif
}

wxTreeItemId CslFrame::TreeFindItem(wxTreeItemId parent,CslGame *game,CslMaster *master)
{
    wxTreeItemId item;
    wxTreeItemIdValue cookie;
    CslTreeItemData *data;

    if (tree_ctrl_games->GetCount()>1)
    {
        item=tree_ctrl_games->GetFirstChild(parent,cookie);
        while (item.IsOk())
        {
            data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
            if (game)
            {
                if (data->GetGame()==game)
                    break;
                else if (master && data->GetMaster()==master)
                    break;
            }
            item=tree_ctrl_games->GetNextSibling(item);
        }
    }

    return item;
}

void CslFrame::TreeAddMaster(wxTreeItemId parent,CslMaster *master,bool focus)
{
    if (!parent.IsOk())
    {
        parent=TreeFindItem(m_treeGamesRoot,master->GetGame(),NULL);
        if (!parent.IsOk())
        {
            TreeAddGame(master->GetGame(),true);
            return;
        }
    }

    if (TreeFindItem(parent,NULL,master).IsOk())
        return;

    wxTreeItemId item;
    CslTreeItemData *data=new CslTreeItemData(master);
    item=tree_ctrl_games->AppendItem(parent,master->GetConnection().GetAddress(),0,-1,data);

    if (focus)
        tree_ctrl_games->SelectItem(item);
}

void CslFrame::TreeRemoveMaster()
{
    wxTreeItemId item;
    CslMaster *master=TreeGetSelectedMaster(&item);

    if (!master)
        return;

    if (m_timer.IsRunning())
        m_timer.Stop();

    master->GetGame()->DeleteMaster(master->GetId());
    tree_ctrl_games->Delete(item);

    m_timer.Start(CSL_TIMER_SHOT);
}

void CslFrame::TreeAddGame(CslGame *game,wxInt32 img,bool focus)
{
    if (!game)
    {
        wxASSERT(game);
        return;
    }

    CslTreeItemData *data;
    wxTreeItemId item=TreeFindItem(m_treeGamesRoot,game,NULL);

    if (!item.IsOk())
    {
        data=new CslTreeItemData(game);
        item=tree_ctrl_games->AppendItem(m_treeGamesRoot,game->GetName(),img,-1,data);
        if (focus)
            tree_ctrl_games->SelectItem(item);
    }

    vector<CslMaster*>& masters=game->GetMasters();
    loopv(masters) TreeAddMaster(item,masters[i]);

    tree_ctrl_games->Expand(item);
}

CslGame* CslFrame::TreeGetSelectedGame(wxTreeItemId *item)
{
    wxTreeItemId id=tree_ctrl_games->GetSelection();

    if (id.IsOk())
    {
        if (item)
            *item=id;

        CslTreeItemData *data;
        if ((data=(CslTreeItemData*)tree_ctrl_games->GetItemData(id))==NULL)
            return NULL;
        if (data->GetMaster())
            return data->GetMaster()->GetGame();
        else
            return data->GetGame();
    }
    return NULL;
}

CslMaster* CslFrame::TreeGetSelectedMaster(wxTreeItemId *item)
{
    wxTreeItemId id=tree_ctrl_games->GetSelection();

    if (id.IsOk())
    {
        if (item)
            *item=id;

        CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(id);
        return data ? data->GetMaster():NULL;
    }
    return NULL;
}

void CslFrame::UpdateMaster()
{
    CslMaster *master=TreeGetSelectedMaster();
    if (!master)
        return;

    SetStatusText(_("Sending request to master: ")+master->GetConnection().GetAddress(),1);
    m_timer.Stop();

    CslMenu::EnableMenuItem(MENU_MASTER_UPDATE,false);
    tree_ctrl_games->Enable(false);

    wxInt32 num=m_engine->UpdateFromMaster(master);
    if (num<0)
        SetStatusText(_("Error on update from master: ")+master->GetConnection().GetAddress(),1);
    else
    {
        m_timerCount=0;
        list_ctrl_master->ListClear();
        SetStatusText(wxString::Format(_("Got %d servers from master"),num),1);
    }

    CslMenu::EnableMenuItem(MENU_MASTER_UPDATE);
    tree_ctrl_games->Enable();

    m_timerInit=true;
    m_timer.Start(CSL_TIMER_SHOT);
}

void CslFrame::ConnectToServer()
{
    if (CslConnectionState::IsPlaying())
    {
        wxMessageBox(_("You are currently playing, so quit the game and try again."),
                     _("Error"),wxOK|wxICON_ERROR,this);
        return;
    }

    CslServerInfo *info=CslConnectionState::GetInfo();
    wxInt32 mode=CslConnectionState::GetConnectMode();

    if (!info)
        return;

    CslConnectionState::Reset();

    if (info->Players>0 && info->Players==info->PlayersMax)
    {
        wxInt32 time=g_cslSettings->waitServerFull;

        CslDlgConnectWait *dlg=new CslDlgConnectWait(this,&time);
        if (dlg->ShowModal()==wxID_OK)
        {
            m_timerCount=0;
            CslConnectionState::CreateWaitingState(info,mode,time);
        }
        return;
    }

    wxString error;
    wxString cmd=info->GetGame().GameStart(info,mode,&error);

    if (cmd.IsEmpty())
    {
        if (!error.IsEmpty())
            wxMessageBox(error,_("Error"),wxICON_ERROR,this);
        return;
    }

    CslDlgOutput::Reset(info->GetBestDescription());

    if (!::wxSetWorkingDirectory(info->GetGame().GetClientSettings().GamePath))
        return;

    info->Lock();

    CslProcess *process=new CslProcess(this,info,cmd);
    if (!(::wxExecute(cmd,wxEXEC_ASYNC,process)>0))
    {
        wxMessageBox(_("Failed to start: ")+cmd,
                     _("Error"),wxICON_ERROR,this);
        info->Lock(false);
        return;
    }

    CslConnectionState::CreatePlayingState(info);

    info->ConnectedTimes++;
    info->PlayLast=wxDateTime::Now().GetTicks();
    list_ctrl_info->UpdateInfo(info);

    list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,true,info);
    list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,true,info);
}

void CslFrame::LoadSettings()
{
    g_cslSettings=new CslSettings;

    wxString file=wxStandardPaths().GetUserDataDir()+PATHDIV+wxT("settings.ini");

    if (!::wxFileExists(file))
        return;

    long int val;
    double dval;
    wxUint32 version;
    wxString s;

    wxFileConfig config(wxEmptyString,wxEmptyString,file,wxEmptyString,wxCONFIG_USE_LOCAL_FILE);

    config.SetPath(wxT("/Version"));
    config.Read(wxT("Version"),&val,0); version=val;

    config.SetPath(wxT("/Gui"));
    if (config.Read(wxT("SizeX"),&val)) g_cslSettings->frameSize.SetWidth(val);
    if (config.Read(wxT("SizeY"),&val)) g_cslSettings->frameSize.SetHeight(val);
    if (config.Read(wxT("Layout"),&s)) g_cslSettings->layout=s;
    if (config.Read(wxT("UpdateInterval"),&val))
    {
        if (val<CSL_UPDATE_INTERVAL_MIN || val>CSL_UPDATE_INTERVAL_MAX)
            val=CSL_UPDATE_INTERVAL_MIN;
        g_cslSettings->updateInterval=val;
    }
    if (config.Read(wxT("NoUpdatePlaying"),&val)) g_cslSettings->dontUpdatePlaying=val!=0;
    if (config.Read(wxT("ShowSearch"),&val)) g_cslSettings->showSearch=val!=0;
    if (config.Read(wxT("FilterMaster"),&val)) g_cslSettings->filterMaster=val;
    if (config.Read(wxT("FilterFavourites"),&val)) g_cslSettings->filterFavourites=val;
    if (config.Read(wxT("WaitOnFullServer"),&val))
    {
        if (val<CSL_WAIT_SERVER_FULL_MIN || val>CSL_WAIT_SERVER_FULL_MAX)
            val=CSL_WAIT_SERVER_FULL_STD;
        g_cslSettings->waitServerFull=val;
    }
    if (config.Read(wxT("PingGood"),&val))
    {
        g_cslSettings->pinggood=val;
        if (g_cslSettings->pinggood>9999)
            g_cslSettings->pinggood=CSL_PING_GOOD_STD;
    }
    if (config.Read(wxT("PingBad"),&val))
    {
        g_cslSettings->pingbad=val;
        if (g_cslSettings->pingbad<g_cslSettings->pinggood)
            g_cslSettings->pingbad=g_cslSettings->pinggood;
    }
    if (config.Read(wxT("GameOutputPath"),&s)) g_cslSettings->gameOutputPath=s;
    if (g_cslSettings->gameOutputPath.IsEmpty())
        g_cslSettings->gameOutputPath=wxStandardPaths().GetUserDataDir();
    if (config.Read(wxT("AutoSaveOutput"),&val)) g_cslSettings->autoSaveOutput=val!=0;
    if (config.Read(wxT("LastSelectedGame"),&s)) g_cslSettings->lastGame=s;
    if (config.Read(wxT("MinPlaytime"),&val))
    {
        if (val<CSL_MIN_PLAYTIME_MIN || val>CSL_MIN_PLAYTIME_MAX)
            val=CSL_MIN_PLAYTIME_STD;
        g_cslSettings->minPlaytime=val;
    }

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    if (config.Read(wxT("AutoSort"),&val)) g_cslSettings->autoSortColumns=val!=0;
    if (version)
    {
        if (config.Read(wxT("ColMult1"),&dval)) g_cslSettings->colServerS1=(float)dval;
        if (config.Read(wxT("ColMult2"),&dval)) g_cslSettings->colServerS2=(float)dval;
        if (config.Read(wxT("ColMult3"),&dval)) g_cslSettings->colServerS3=(float)dval;
        if (config.Read(wxT("ColMult4"),&dval)) g_cslSettings->colServerS4=(float)dval;
        if (config.Read(wxT("ColMult5"),&dval)) g_cslSettings->colServerS5=(float)dval;
        if (config.Read(wxT("ColMult6"),&dval)) g_cslSettings->colServerS6=(float)dval;
        if (config.Read(wxT("ColMult7"),&dval)) g_cslSettings->colServerS7=(float)dval;
        if (config.Read(wxT("ColMult8"),&dval)) g_cslSettings->colServerS8=(float)dval;
        if (config.Read(wxT("ColMult9"),&dval)) g_cslSettings->colServerS9=(float)dval;
    }
    if (config.Read(wxT("ColourEmpty"),&val)) g_cslSettings->colServerEmpty=INT2COLOUR(val);
    if (config.Read(wxT("ColourOffline"),&val)) g_cslSettings->colServerOff=INT2COLOUR(val);
    if (config.Read(wxT("ColourFull"),&val)) g_cslSettings->colServerFull=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM1"),&val)) g_cslSettings->colServerMM1=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM2"),&val)) g_cslSettings->colServerMM2=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM3"),&val)) g_cslSettings->colServerMM3=INT2COLOUR(val);
    if (config.Read(wxT("ColourSearch"),&val)) g_cslSettings->colServerHigh=INT2COLOUR(val);
    if (config.Read(wxT("ColourPlaying"),&val)) g_cslSettings->colServerPlay=INT2COLOUR(val);
    if (config.Read(wxT("ColourStripes"),&val)) g_cslSettings->colInfoStripe=INT2COLOUR(val);

    /* Client */
    vector<CslGame*>& games=m_engine->GetGames();
    loopv(games)
    {
#ifdef __WXMAC__
        val=true;
#else
        val=false;
#endif //__WXMAC__
        CslGameClientSettings& settings=games[i]->GetClientSettings();

        config.SetPath(wxT("/")+games[i]->GetName());
        if (config.Read(wxT("Binary"),&s)) settings.Binary=s;
        if (config.Read(wxT("GamePath"),&s)) settings.GamePath=s;
        if (config.Read(wxT("ConfigPath"),&s))
        {
            if (s.IsEmpty() && !settings.GamePath.IsEmpty())
                settings.ConfigPath=settings.GamePath;
            else if (!s.IsEmpty() && ::wxDirExists(s))
                settings.ConfigPath=s;
        }
        if (config.Read(wxT("Options"),&s)) settings.Options=s;
        if (config.Read(wxT("ExpertConfig"),&val)) settings.Expert=val!=0;
        if (config.Read(wxT("PortDelimiter"),&s)) games[i]->SetPortDelimiter(s);
        if (!settings.Binary.IsEmpty() && !::wxFileExists(settings.Binary))
            settings.Binary=wxEmptyString;
        if (!settings.GamePath.IsEmpty() && !::wxDirExists(settings.GamePath))
            settings.GamePath=wxEmptyString;
    }
}

void CslFrame::SaveSettings()
{
    wxString dir=wxStandardPaths().GetUserDataDir();

    if (!::wxDirExists(dir))
        ::wxMkdir(dir,0700);

    wxFileConfig config(wxEmptyString,wxEmptyString,dir+PATHDIV+wxT("settings.ini"),
                        wxEmptyString,wxCONFIG_USE_LOCAL_FILE);
    config.SetUmask(0077);
    config.DeleteAll();

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"),CSL_CONFIG_VERSION);

    /* GUI */
    wxSize size=GetSize();
    config.SetPath(wxT("/Gui"));
    config.Write(wxT("SizeX"),(long int)size.GetWidth());
    config.Write(wxT("SizeY"),(long int)size.GetHeight());
    config.Write(wxT("Layout"),m_AuiMgr.SavePerspective());
    config.Write(wxT("UpdateInterval"),(long int)g_cslSettings->updateInterval);
    config.Write(wxT("NoUpdatePlaying"),g_cslSettings->dontUpdatePlaying);
    config.Write(wxT("ShowSearch"),g_cslSettings->showSearch);
    config.Write(wxT("FilterMaster"),(long int)g_cslSettings->filterMaster);
    config.Write(wxT("FilterFavourites"),(long int)g_cslSettings->filterFavourites);
    config.Write(wxT("WaitOnFullServer"),(long int)g_cslSettings->waitServerFull);
    config.Write(wxT("PingGood"),(long int)g_cslSettings->pinggood);
    config.Write(wxT("PingBad"),(long int)g_cslSettings->pingbad);
    config.Write(wxT("GameOutputPath"),g_cslSettings->gameOutputPath);
    config.Write(wxT("AutoSaveOutput"),g_cslSettings->autoSaveOutput);
    config.Write(wxT("LastSelectedGame"),g_cslSettings->lastGame);
    config.Write(wxT("MinPlaytime"),(long int)g_cslSettings->minPlaytime);

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    config.Write(wxT("AutoSort"),g_cslSettings->autoSortColumns);
    config.Write(wxT("ColMult1"),g_cslSettings->colServerS1);
    config.Write(wxT("ColMult2"),g_cslSettings->colServerS2);
    config.Write(wxT("ColMult3"),g_cslSettings->colServerS3);
    config.Write(wxT("ColMult4"),g_cslSettings->colServerS4);
    config.Write(wxT("ColMult5"),g_cslSettings->colServerS5);
    config.Write(wxT("ColMult6"),g_cslSettings->colServerS6);
    config.Write(wxT("ColMult7"),g_cslSettings->colServerS7);
    config.Write(wxT("ColMult8"),g_cslSettings->colServerS8);
    config.Write(wxT("ColMult9"),g_cslSettings->colServerS9);
    config.Write(wxT("ColourEmpty"),COLOUR2INT(g_cslSettings->colServerEmpty));
    config.Write(wxT("ColourOffline"),COLOUR2INT(g_cslSettings->colServerOff));
    config.Write(wxT("ColourFull"),COLOUR2INT(g_cslSettings->colServerFull));
    config.Write(wxT("ColourMM1"),COLOUR2INT(g_cslSettings->colServerMM1));
    config.Write(wxT("ColourMM2"),COLOUR2INT(g_cslSettings->colServerMM2));
    config.Write(wxT("ColourMM3"),COLOUR2INT(g_cslSettings->colServerMM3));
    config.Write(wxT("ColourSearch"),COLOUR2INT(g_cslSettings->colServerHigh));
    config.Write(wxT("ColourPlaying"),COLOUR2INT(g_cslSettings->colServerPlay));
    config.Write(wxT("ColourStripes"),COLOUR2INT(g_cslSettings->colInfoStripe));

    /* Client */
    vector<CslGame*>& games=m_engine->GetGames();
    loopv(games)
    {
        CslGame& game=*games[i];
        const CslGameClientSettings& settings=game.GetClientSettings();

        config.SetPath(wxT("/")+game.GetName());
        config.Write(wxT("Binary"),settings.Binary);
        config.Write(wxT("GamePath"),settings.GamePath);
        config.Write(wxT("ConfigPath"),settings.ConfigPath);
        config.Write(wxT("Options"),settings.Options);
        config.Write(wxT("ExpertConfig"),settings.Expert);
        config.Write(wxT("PortDelimiter"),game.GetPortDelimiter());
    }
}

bool CslFrame::LoadServers(wxUint32 *numm,wxUint32 *nums)
{
    wxString file=wxStandardPaths().GetUserDataDir()+PATHDIV+wxT("servers.ini");

    if (!::wxFileExists(file))
        return false;

    long int val;
    bool read_server;
    wxUint32 mc=0,sc=0,tmc=0,tsc=0;
    CslMaster *master;
    vector<CslIDMapping*> mappings;
    vector<wxInt32> ids;

    wxString addr,path,pass1,pass2,description;
    wxUint16 port=0;
    wxUint32 view=0;
    wxUint32 lastSeen=0;
    wxUint32 playLast=0;
    wxUint32 playTimeLastGame=0;
    wxUint32 playTimeTotal=0;
    wxUint32 connectedTimes=0;
    wxInt32 gt;

    wxFileConfig config(wxEmptyString,wxEmptyString,file,wxEmptyString,wxCONFIG_USE_LOCAL_FILE);

    vector<CslGame*>& games=m_engine->GetGames();

    for (gt=0;gt<games.length();gt++)
    {
        wxString s=wxT("/")+games[gt]->GetName();
        config.SetPath(s);

        mc=0;
        while (1)
        {
            long int id;
            CslMasterConnection connection;
            master=NULL;

            config.SetPath(s+wxString::Format(wxT("/Master/%d"),mc++));

            if (config.Read(wxT("Address"),&addr))
            {
                if (config.Read(wxT("ID"),&id))
                {
                    val=0;
                    config.Read(wxT("Port"),&val,0); port=val;
                    config.Read(wxT("Type"),&val,CslMasterConnection::CONNECTION_HTTP);
                    if (val==CslMasterConnection::CONNECTION_HTTP)
                    {
                        port=port ? port:CSL_DEFAULT_MASTER_WEB_PORT;
                        if (config.Read(wxT("Path"),&path))
                            master=new CslMaster(CslMasterConnection(addr,path,port));
                    }
                    else if (port)
                        master=new CslMaster(CslMasterConnection(addr,port));
                }
            }
            if (!master)
                break;

            if (games[gt]->AddMaster(master)<0)
            {
                delete master;
                continue;
            }
            mappings.add(new CslIDMapping(id,master->GetId()));
            tmc++;
        }

        sc=0;
        read_server=true;
        while (read_server)
        {
            wxInt32 id=0;
            ids.setsize(0);

            config.SetPath(s+wxString::Format(wxT("/Server/%d"),sc++));

            read_server=false;
            if (config.Read(wxT("Address"),&addr))
            {
                config.Read(wxT("Port"),&val,games[gt]->GetDefaultPort()); port=val;
                config.Read(wxT("Password"),&pass1,wxEmptyString);
                config.Read(wxT("AdminPassword"),&pass2,wxEmptyString);
                config.Read(wxT("Description"),&description,wxEmptyString);

                while (config.Read(wxString::Format(wxT("Master%d"),id++),&val))
                    ids.add(val);
                if (ids.length()==0)
                    ids.add(-1);

                if (config.Read(wxT("View"),&val))
                {
                    view=val;
                    config.Read(wxT("LastSeen"),&val,0);
                    lastSeen=(wxUint32)val;
                    config.Read(wxT("PlayLast"),&val,0);
                    playLast=(wxUint32)val;
                    config.Read(wxT("PlayTimeLastGame"),&val,0);
                    playTimeLastGame=(wxUint32)val;
                    config.Read(wxT("PlayTimeTotal"),&val,0);
                    playTimeTotal=(wxUint32)val;
                    config.Read(wxT("ConnectedTimes"),&val,0);
                    connectedTimes=(wxUint32)val;
                    read_server=true;
                }
            }

            if (!read_server)
                break;

            loopv(ids)
            {
                CslServerInfo *info=new CslServerInfo(games[gt],addr,port,view,
                                                      lastSeen,playLast,playTimeLastGame,
                                                      playTimeTotal,connectedTimes,
                                                      description,pass1,pass2);

                if (ids[i]==-1)
                    id=-1;
                else
                {
                    loopvj(mappings)
                    {
                        if (mappings[j]->m_oldId==ids[i])
                        {
                            id=mappings[j]->m_newId;
                            break;
                        }
                    }
                }

                if (games[gt]->AddServer(info,id))
                {
                    m_engine->ResolveHost(info);

                    long type;
                    wxInt32 pos,j=0;
                    wxString guiview;

                    while (config.Read(wxString::Format(wxT("GuiView%d"),j++),&guiview))
                    {
                        if ((pos=guiview.Find(wxChar(':')))<0)
                            continue;
                        if (!guiview.Mid(0,pos).ToLong(&type))
                            continue;
                        if ((guiview=guiview.Mid(pos+1)).IsEmpty())
                            continue;
                        PlayerListCreateView(info,type,guiview);
                    }

                    tsc++;
                }
                else
                    delete info;
            }
        }
        //m_engine->ResetPingSends(games[gt],NULL);
    }

    loopv(mappings)
    delete mappings[i];

    if (numm)
        *nums=tmc;
    if (nums)
        *nums=tsc;

    return tmc>0||tsc>0;
}

void CslFrame::SaveServers()
{
    wxString s=wxStandardPaths().GetUserDataDir();

    if (!::wxDirExists(s))
        if (!::wxMkdir(s,0700))
            return;

    wxFileConfig config(wxEmptyString,wxEmptyString,s+PATHDIV+wxT("servers.ini"),
                        wxEmptyString,wxCONFIG_USE_LOCAL_FILE);
    config.SetUmask(0077);
    config.DeleteAll();

    wxInt32 mc,sc;
    CslGame *game;
    CslMaster *master;
    CslServerInfo *info;

    vector<CslGame*>& games=m_engine->GetGames();

    if (!games.length())
        return;

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"),CSL_SERVERCONFIG_VERSION);

    loopv(games)
    {
        game=games[i];
        s=wxT("/")+game->GetName();
        config.SetPath(s);

        vector<CslMaster*>& masters=game->GetMasters();
        mc=0;

        loopvj(masters)
        {
            master=masters[j];
            CslMasterConnection& connection=master->GetConnection();

            config.SetPath(s+s.Format(wxT("/Master/%d"),mc++));
            config.Write(wxT("Type"),connection.GetType());
            config.Write(wxT("Address"),connection.GetAddress());
            config.Write(wxT("Port"),connection.GetPort());
            config.Write(wxT("Path"),connection.GetPath());
            config.Write(wxT("ID"),master->GetId());
        }

        vector<CslServerInfo*>& servers=game->GetServers();
        if (servers.length()==0)
            continue;

        sc=0;
        loopvj(servers)
        {
            info=servers[j];
            config.SetPath(s+s.Format(wxT("/Server/%d"),sc++));
            config.Write(wxT("Address"),info->Host);
            config.Write(wxT("Port"),info->Port);
            config.Write(wxT("Password"),info->Password);
            config.Write(wxT("AdminPassword"),info->PasswordAdmin);
            config.Write(wxT("Description"),info->DescriptionOld);
            const vector<wxInt32> masters=info->GetMasterIDs();
            loopvk(masters) config.Write(wxString::Format(wxT("Master%d"),k),masters[k]);
            config.Write(wxT("View"),(int)info->View);
            config.Write(wxT("LastSeen"),(int)info->LastSeen);
            config.Write(wxT("PlayLast"),(int)info->PlayLast);
            config.Write(wxT("PlayTimeLastGame"),(int)info->PlayTimeLastGame);
            config.Write(wxT("PlayTimeTotal"),(int)info->PlayTimeTotal);
            config.Write(wxT("ConnectedTimes"),(int)info->ConnectedTimes);

            wxUint32 l=0;
            loopvk(m_playerLists)
            {
                if (k==0)
                    continue;
                CslListCtrlPlayer *list=m_playerLists[k];
                if (list->GetInfo()==info)
                {
                    wxAuiPaneInfo& pane=m_AuiMgr.GetPane(list);
                    if (!pane.IsOk())
                        continue;

                    wxString view=wxString::Format(wxT("%d:"),list->GetView())+pane.name;
                    config.Write(wxString::Format(wxT("GuiView%d"),l++),view);
                }
            }
        }
    }
}

void CslFrame::OnPong(wxCommandEvent& event)
{
    CslPongPacket *packet=(CslPongPacket*)event.GetClientData();

    if (!packet)
        return;

    if (packet->Type==CSL_PONG_TYPE_PING)
    {
        delete packet;
        return;
    }

    if (m_extendedDlg->GetInfo()==packet->Info && m_extendedDlg->IsShown())
    {
        switch (packet->Type)
        {
            case CSL_PONG_TYPE_PLAYERSTATS:
                m_extendedDlg->UpdatePlayerData();
                break;

            case CSL_PONG_TYPE_TEAMSTATS:
                m_extendedDlg->UpdateTeamData();
                break;

            default:
                break;
        }
    }

    if (packet->Type==CSL_PONG_TYPE_PLAYERSTATS)
    {
        loopv(m_playerLists)
        {
            if (m_playerLists[i]->GetInfo()!=packet->Info)
                continue;

            switch (packet->Type)
            {
                case CSL_PONG_TYPE_PLAYERSTATS:
                {
                    m_playerLists[i]->ListUpdatePlayerData(packet->Info->GetGame(),packet->Info->PlayerStats);
                    break;
                }

                default:
                    break;
            }
        }

        if (packet->Type==CSL_PONG_TYPE_PLAYERSTATS &&
            m_searchedServers.length() &&
            radio_search_player->GetValue())
        {
            wxInt32 progress;
            bool found;

            loopv(m_searchedServers)
            {
                found=false;

                if (m_searchedServers[i]==packet->Info)
                {
                    if (!packet->Info->Search)
                        continue;

                    packet->Info->Search=false;

                    progress=gauge_search->GetValue()+1;
                    gauge_search->SetValue(progress);

                    CslPlayerStats& stats=m_searchedServers[i]->PlayerStats;
                    loopvj(stats.m_stats)
                    {
                        CslPlayerStatsData *data=stats.m_stats[j];
                        if (data->Ok && data->Name.Lower().Contains(m_searchString))
                        {
                            list_ctrl_master->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,true,packet->Info);
                            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,true,packet->Info);
                            m_searchResultPlayer++;
                            found=true;
                        }
                    }

                    if (progress==m_searchedServers.length())
                    {
                        loopv(m_searchedServers) m_searchedServers[i]->SetPingExt(false);
                        m_searchedServers.setsize(0);
                    }

                    if (found)
                        m_searchResultServer++;
                }
            }

            text_search_result->SetLabel(wxString::Format(_("Search result: %d players on %d servers"),
                                         m_searchResultPlayer,m_searchResultServer));
            sizer_search->Layout();
        }

    }

    delete packet;
}

void CslFrame::OnTimer(wxTimerEvent& event)
{
    static wxUint32 lightCount=0;

    if (!(g_cslSettings->dontUpdatePlaying && CslConnectionState::IsPlaying()))
    {
        CslGame *game=TreeGetSelectedGame();
        CslMaster *master=TreeGetSelectedMaster();

        if (m_timerCount>=m_timerUpdate)
        {
            m_timerCount=0;

            if (game)
            {
                list_ctrl_master->ListUpdate(master? master->GetServers():game->GetServers());

                vector<CslServerInfo*> servers;
                m_engine->GetFavourites(servers);
                list_ctrl_favourites->ListUpdate(servers);
            }
        }
        else
            m_timerCount++;

        bool green=game && m_engine->PingServers(game,m_timerInit);
        green|=m_engine->PingServersEx()!=0;

        if (green && !CslConnectionState::IsWaiting())
            CslStatusBar::Light(LIGHT_GREEN);

        if (m_timerInit)
        {
            m_timerInit=false;
            m_timerCount=m_timerUpdate-(1000/CSL_TIMER_SHOT);
            m_engine->ResetPingSends(game,NULL);
        }
        else
            m_engine->CheckResends();

        if (m_trafficDlg)
            wxPostEvent(m_trafficDlg,event);
    }
    else
        m_timerCount++;

    if (CslConnectionState::IsPlaying() && m_timerCount%2==0)
    {
        CslProcess::ProcessInputStream();
        CslProcess::ProcessErrorStream();

        CslStatusBar::Light(LIGHT_YELLOW);
    }
    else if (CslConnectionState::IsWaiting())
    {
        CslServerInfo *info=CslConnectionState::GetInfo();

        if (info->Players!=info->PlayersMax)
            ConnectToServer();
        else if (m_timerCount%4==0)
        {
            if (CslConnectionState::CountDown())
                CslStatusBar::SetText(1,wxString::Format(_("Waiting %s for a free slot on \"%s\" " \
                                      "(press ESC to abort or join another server)"),
                                      FormatSeconds(CslConnectionState::GetWaitTime(),true,true).c_str(),
                                      info->GetBestDescription().c_str()));
            else
                CslStatusBar::SetText(1,wxT(""));
        }

        CslStatusBar::Light(LIGHT_RED);
    }

    if (CslStatusBar::Light()!=LIGHT_GREY)
    {
        if ((lightCount=++lightCount%3)==0)
            CslStatusBar::Light(LIGHT_GREY);
    }
}

void CslFrame::OnListItemSelected(wxListEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    if (!info)
        return;

    list_ctrl_info->UpdateInfo(info);

    if (m_oldSelectedInfo)
        m_oldSelectedInfo->SetPingExt(false);
    m_oldSelectedInfo=info;

    list_ctrl_players->SetInfo(info);

    info->SetPingExt(true);

    if (!m_engine->PingEx(info))
        list_ctrl_players->ListUpdatePlayerData(info->GetGame(),info->PlayerStats);

    {
        wxAuiPaneInfo& pane=m_AuiMgr.GetPane(list_ctrl_info);
        if (pane.IsOk())
            pane.Caption(wxString(CSL_CAPTION_SERVER_INFO)+wxT(": ")+info->GetBestDescription());
    }
    {
        wxAuiPaneInfo& pane=m_AuiMgr.GetPane(list_ctrl_players);
        if (pane.IsOk())
            pane.Caption(PlayerListGetCaption(info,true));
    }
    m_AuiMgr.Update();
}

void CslFrame::OnListItemActivated(wxListEvent& event)
{
    if (event.GetEventObject()==(void*)list_ctrl_info)
        return;
    ConnectToServer();
}

void CslFrame::OnTreeSelChanged(wxTreeEvent& event)
{
    event.Skip();

    wxTreeItemId item=event.GetItem();
    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);

    CslGame *game=data->GetGame();
    CslMaster *master=data->GetMaster();

    vector<CslServerInfo*>* servers;

    if (!game)
    {
        CslMenu::EnableMenuItem(MENU_MASTER_DEL);
        CslMenu::EnableMenuItem(MENU_MASTER_UPDATE);

        servers=&master->GetServers();

        //find the game
        item=tree_ctrl_games->GetItemParent(item);
        if (!item.IsOk())
        {
            wxASSERT(item.IsOk());
            return;
        }
        data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
        game=data->GetGame();
    }
    else
    {
        CslMenu::EnableMenuItem(MENU_MASTER_DEL,false);
        CslMenu::EnableMenuItem(MENU_MASTER_UPDATE,false);
        servers=&game->GetServers();
    }

    if (game)
        g_cslSettings->lastGame=game->GetName();

    list_ctrl_master->ListClear();
    SetTotalPlaytime(game);

    list_ctrl_master->SetMasterSelected(master!=NULL);
    list_ctrl_master->ListUpdate(*servers);

    m_timerCount=0;
    m_timerInit=true;
}

void CslFrame::OnTreeRightClick(wxTreeEvent& event)
{
    event.Skip();

    wxTreeItemId item=event.GetItem();
    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);

    CslMaster *master=data->GetMaster();

    if (master)
    {
        CslMenu::EnableMenuItem(MENU_MASTER_DEL);
        CslMenu::EnableMenuItem(MENU_MASTER_UPDATE);
    }
    else
    {
        CslMenu::EnableMenuItem(MENU_MASTER_DEL,false);
        CslMenu::EnableMenuItem(MENU_MASTER_UPDATE,false);
    }

    PopupMenu(menuMaster);
}

void CslFrame::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxID_EXIT:
            this->Destroy();
            break;

        case wxID_PREFERENCES:
        {
            CslDlgSettings *dlg=new CslDlgSettings(m_engine,(wxWindow*)this);
            if (dlg->ShowModal()!=wxID_OK)
                break;
            m_timerUpdate=g_cslSettings->updateInterval/CSL_TIMER_SHOT;
            m_engine->SetUpdateInterval(g_cslSettings->updateInterval);
            SaveSettings();
            break;
        }

        case MENU_MASTER_ADD:
        {
            wxInt32 gameId=TreeGetSelectedGame()->GetId();
            CslDlgAddMaster dlg((wxWindow*)this);
            dlg.InitDlg(m_engine,&gameId);
            if (dlg.ShowModal()==wxID_OK)
            {
                CslGame *game=m_engine->GetGames()[gameId];
                CslMaster *master=game->GetMasters().last();
                game->AddMaster(master);
                TreeAddMaster(wxTreeItemId(),master,true);
            }
            break;
        }

        case MENU_MASTER_DEL:
        {
            list_ctrl_master->ListClear();
            TreeRemoveMaster();
            break;
        }

        case MENU_MASTER_UPDATE:
            UpdateMaster();
            break;

        case MENU_VIEW_GAMES:
        case MENU_VIEW_SERVER_INFO:
        case MENU_VIEW_PLAYER_LIST:
        case MENU_VIEW_FAVOURITES:
            TogglePane(event.GetId());
            break;

        case MENU_VIEW_SEARCH:
            g_cslSettings->showSearch=event.IsChecked();
            ToggleSearchBar();
            break;

        case MENU_VIEW_OUTPUT:
            m_outputDlg->Show(!m_outputDlg->IsShown());
            break;

        case MENU_VIEW_AUTO_SORT:
            g_cslSettings->autoSortColumns=event.IsChecked();
            list_ctrl_master->ListSort();
            list_ctrl_master->ToggleSortArrow();
            list_ctrl_favourites->ListSort();
            list_ctrl_favourites->ToggleSortArrow();
            break;

        case MENU_VIEW_TRAFFIC:
        {
            m_trafficDlg=new CslDlgTraffic(this);
            m_trafficDlg->ShowModal();
            break;
        }

        case wxID_ABOUT:
        {
            CslDlgAbout *dlg=new CslDlgAbout(this);
            dlg->ShowModal();
            break;
        }

        case CSL_BUTTON_SEARCH_CLOSE:
            g_cslSettings->showSearch=false;
            CslMenu::CheckMenuItem(MENU_VIEW_SEARCH,false);
            ToggleSearchBar();
            break;

        case CSL_TEXT_SEARCH:
        {
            wxString s=event.GetString();

            if (s.IsEmpty())
            {
                loopv(m_searchedServers) m_searchedServers[i]->SetPingExt(false);
                m_searchedServers.setsize(0);
                m_searchString.Empty();
                button_search->Enable(false);
                gauge_search->Enable(false);
                gauge_search->SetValue(0);
                SetSearchbarColour(false);
                wxInt32 type=-1;
                list_ctrl_master->Highlight(type,false);
                list_ctrl_favourites->Highlight(type,false);
                list_ctrl_master->ListSearch(s);
                list_ctrl_favourites->ListSearch(s);
                text_search_result->SetLabel(wxString::Format(_("Search result: %d servers"),0));
            }
            else
            {
                button_search->Enable();
                gauge_search->Enable();
            }

            if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED)
            {
                if (radio_search_server->GetValue())
                {
                    wxInt32 sr=list_ctrl_master->ListSearch(s);
                    sr+=list_ctrl_favourites->ListSearch(s);

                    SetSearchbarColour(!(sr || s.IsEmpty()));

                    text_search_result->SetLabel(wxString::Format(_("Search result: %d servers"),sr));
                    sizer_search->Layout();
                }
                else if (radio_search_player->GetValue())
                    m_searchString=text_ctrl_search->GetValue().Lower();
                break;
            }
            else if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;
        }

        case CSL_BUTTON_SEARCH:
        {
            m_searchResultPlayer=m_searchResultServer=0;
            loopv(m_searchedServers) m_searchedServers[i]->SetPingExt(false);
            m_searchedServers.setsize(0);
            gauge_search->SetValue(0);
            list_ctrl_master->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,false);
            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,false);
            text_search_result->SetLabel(wxString::Format(_("Search result: %d players on %d servers"),0,0));

            if (text_ctrl_search->GetValue().IsEmpty())
                break;

            CslGame *game=TreeGetSelectedGame();
            if (!game)
                break;

            vector<CslServerInfo*>& servers=game->GetServers();
            loopv(servers)
            {
                if (CslEngine::PingOk(*servers[i],g_cslSettings->updateInterval) &&
                    servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
                {
                    m_searchedServers.add(servers[i]);
                    servers[i]->Search=true;
                    servers[i]->SetPingExt(true);
                }
            }

            gauge_search->SetRange(m_searchedServers.length());

            m_engine->PingServersEx(true);
            break;
        }

        case CSL_RADIO_SEARCH_SERVER:
        {
            button_search->Hide();
            gauge_search->Hide();
            sizer_search->Layout();
            text_ctrl_search->SetFocus();
            text_ctrl_search->Clear();
#ifdef __WXMAC__
            wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED,CSL_TEXT_SEARCH);
            wxPostEvent(this,evt);
#endif
            break;
        }

        case CSL_RADIO_SEARCH_PLAYER:
        {
            button_search->Show();
            gauge_search->Show();
            sizer_search->Layout();
            text_ctrl_search->SetFocus();
            text_ctrl_search->Clear();
#ifdef __WXMAC__
            wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED,CSL_TEXT_SEARCH);
            wxPostEvent(this,evt);
#endif
            break;
        }

        //events from the server lists
        case MENU_SERVER_CONNECT:
        case MENU_SERVER_CONNECT_PW:
            ConnectToServer();
            break;

        case MENU_SERVER_DEL:
        {
            CslServerInfo *info=(CslServerInfo*)event.GetClientData();
            if (!info)
                break;

            if (info==m_oldSelectedInfo)
                m_oldSelectedInfo=NULL;

            if (m_extendedDlg->GetInfo()==info)
                info->SetPingExt(false);

            loopvrev(m_playerLists)
            {
                if (m_playerLists[i]->GetInfo()==info)
                {
                    if (i>0 && m_AuiMgr.DetachPane(m_playerLists[i]))
                    {
                        m_AuiMgr.Update();

                        delete m_playerLists[i];
                        m_playerLists.remove(i);
                    }
                    info->SetPingExt(false);
                }
            }

            info->GetGame().DeleteServer(info);
            break;
        }

        case MENU_SERVER_EXTENDED_MICRO:
        case MENU_SERVER_EXTENDED_MINI:
        case MENU_SERVER_EXTENDED_DEFAULT:
        {
            CslServerInfo *info=(CslServerInfo*)event.GetClientData();
            if (info)
                PlayerListCreateView(info,event.GetId()-MENU_SERVER_EXTENDED_MICRO);
            break;
        }
        case MENU_SERVER_EXTENDED_FULL:
        {
            CslServerInfo *info=(CslServerInfo*)event.GetClientData();
            if (info)
            {
                info->SetPingExt(true);
                m_extendedDlg->DoShow(info);
            }
            break;
        }

        case MENU_SERVER_FILTER_OFF:
        case MENU_SERVER_FILTER_FULL:
        case MENU_SERVER_FILTER_EMPTY:
        case MENU_SERVER_FILTER_NONEMPTY:
        case MENU_SERVER_FILTER_MM2:
        case MENU_SERVER_FILTER_MM3:
        case MENU_SERVER_FILTER_VER:
            SetListCaption((wxInt32)(long)event.GetClientData());
            break;

        default:
            break;
    }
}

void CslFrame::OnKeypress(wxKeyEvent& event)
{
    event.Skip();

    static wxUint32 lastTicks=0;
    wxUint32 ticks=GetTicks();

    if (ticks-lastTicks<500)
        return;
    lastTicks=ticks;

    if (event.GetKeyCode()==WXK_ESCAPE)
    {
        if (CslConnectionState::IsWaiting())
            ConnectToServer();
        else if (FindFocus()==text_ctrl_search)
        {
            text_ctrl_search->Clear();
            wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED,CSL_TEXT_SEARCH);
            wxPostEvent(this,evt);
            return;
        }
    }
}

void CslFrame::OnShow(wxShowEvent& event)
{
    list_ctrl_master->ListAdjustSize(list_ctrl_master->GetClientSize());
    list_ctrl_favourites->ListAdjustSize(list_ctrl_favourites->GetClientSize());
}

void CslFrame::OnClose(wxCloseEvent& event)
{
    if (event.GetEventObject()==m_extendedDlg)
    {
        LOG_DEBUG("\n");
    }
    if (event.GetEventObject()==m_outputDlg)
    {
        CslMenu::CheckMenuItem(MENU_VIEW_OUTPUT,false);
        return;
    }
    if (event.GetEventObject()==m_trafficDlg)
    {
        m_trafficDlg->Destroy();
        m_trafficDlg=NULL;
        return;
    }

    if (CslConnectionState::IsPlaying())
    {
        wxMessageBox(_("There is currently a game running.\n"
                       "Quit the game first and try again."),
                     _("Error"),wxOK|wxICON_ERROR,this);
        return;
    }

    CslDlgGeneric *dlg=new CslDlgGeneric(this,CSL_DLG_GENERIC_DEFAULT,_("CSL terminating"),
                                         _("Waiting for engine to terminate ..."),
                                         wxArtProvider::GetBitmap(wxART_INFORMATION,wxART_CMN_DIALOG));
    dlg->Show();
    wxYield();

    event.Skip();
}

void CslFrame::OnPaneClose(wxAuiManagerEvent& event)
{
    CslServerInfo *info;

    if (event.pane->name.StartsWith(wxT("players")))
    {
        loopv(m_playerLists)
        {
            if (m_playerLists[i]==event.pane->window)
            {
                if ((info=m_playerLists[i]->GetInfo())!=NULL)
                    info->SetPingExt(false);
                if (i==0)
                {
                    CslMenu::CheckMenuItem(MENU_VIEW_PLAYER_LIST,false);
                    m_playerLists[i]->SetInfo(NULL);
                }
                else
                    m_playerLists.remove(i);
                break;
            }
        }
    }
    else if (event.pane->name==(wxT("games")))
        CslMenu::CheckMenuItem(MENU_VIEW_GAMES,false);
    else if (event.pane->name==(wxT("info")))
        CslMenu::CheckMenuItem(MENU_VIEW_SERVER_INFO,false);
    else if (event.pane->name==(wxT("favlist")))
        CslMenu::CheckMenuItem(MENU_VIEW_FAVOURITES,false);
}

void CslFrame::OnMouseEnter(wxMouseEvent& event)
{
    event.Skip();

    button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(1));
    button_search_clear->Refresh();
    // no event id on wxMAC
    /*switch (event.GetId())
    {
        case CSL_BUTTON_SEARCH_CLOSE:
            button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(1));
            break;

        default:
            break;
    }*/
}

void CslFrame::OnMouseLeave(wxMouseEvent& event)
{
    event.Skip();

    button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(0));
    button_search_clear->Refresh();
    // no event id on wxGTK
    /*switch (event.GetId())
    {
        case CSL_BUTTON_SEARCH_CLEAR:
            button_search_clear->SetBitmapLabel(close_14_xpm);
            break;

        default:
            break;
    }*/
}

void CslFrame::OnMouseLeftDown(wxMouseEvent& event)
{
    event.Skip();

    switch (event.GetId())
    {
        case CSL_BUTTON_SEARCH_CLOSE:
            button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(2));
            button_search_clear->Refresh();
            break;

        default:
            break;
    }
}

void CslFrame::OnVersionCheck(wxCommandEvent& event)
{
    wxString *version=(wxString*)event.GetClientData();

    if (version)
    {
        if (version->Cmp(CSL_VERSION_STR)>0)
        {
            CslDlgGeneric *dlg=new CslDlgGeneric(this,CSL_DLG_GENERIC_URL|CSL_DLG_GENERIC_CLOSE,
                                                 wxString::Format(_("New version %s"),version->c_str()),
                                                 wxString::Format(_("There is a new version (%s)\n"\
                                                                    "of Cube Server Lister available.\n"),
                                                                  version->c_str()),
                                                 wxArtProvider::GetBitmap(wxART_INFORMATION,wxART_CMN_DIALOG),
                                                 CSL_WEBADDRFULL_STR);
            dlg->Show();
        }

        delete version;
    }

    if (!m_versionCheckThread)
        return;

    m_versionCheckThread->Delete();
    delete m_versionCheckThread;
    m_versionCheckThread=NULL;
}

void CslFrame::OnEndProcess(wxCommandEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    info->Lock(false);
    info->GetGame().GameEnd();
    CslConnectionState::Reset();

    list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,info);
    list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,info);
    list_ctrl_info->UpdateInfo(info);
}


IMPLEMENT_APP(CslApp)

bool CslApp::OnInit()
{
    ::wxSetWorkingDirectory(wxPathOnly(wxTheApp->argv[0]));
    g_basePath=::wxGetCwd();

    m_locale.Init(wxLANGUAGE_DEFAULT,wxLOCALE_CONV_ENCODING);
    m_locale.AddCatalogLookupPathPrefix(LOCALEPATH);
#ifdef __WXGTK__
    m_locale.AddCatalogLookupPathPrefix(g_basePath+wxString(wxT("/lang")));
#endif
    m_locale.AddCatalog(CSL_NAME_SHORT_STR);

#ifdef __WXMAC__
    wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"),1);
    //enables Command-H, Command-M and Command-Q at least when not in fullscreen
    wxSetEnv(wxT("SDL_SINGLEDISPLAY"),wxT("1"));
    wxSetEnv(wxT("SDL_ENABLEAPPEVENTS"),wxT("1"));
    //TODO wxApp::SetExitOnFrameDelete(false);
#endif

    wxString name=wxString::Format(wxT(".%s-%s.lock"),CSL_NAME_SHORT_STR,wxGetUserId().c_str());
    m_single=new wxSingleInstanceChecker(name);

    if (false &&m_single->IsAnotherRunning())
    {
        CslDlgGeneric *dlg=new CslDlgGeneric(NULL,CSL_DLG_GENERIC_DEFAULT,
                                             _("CSL already running"),
                                             _("CSL is already running."),
                                             wxArtProvider::GetBitmap(wxART_INFORMATION,wxART_CMN_DIALOG));
        dlg->Show();
        dlg->Update();
        SetTopWindow(dlg);
        Yield();
        wxSleep(3);
        dlg->Destroy();

        delete dlg;
        delete m_single;

        return false;
    }

    CslGeoIP::Init();

    wxInitAllImageHandlers();
    CslFrame* frame_csl=new CslFrame(NULL,wxID_ANY,wxEmptyString);
    frame_csl->Show();
    SetTopWindow(frame_csl);

    return true;
}

int CslApp::OnExit()
{
    CslGeoIP::Destroy();
    delete m_single;

    return 0;
}
