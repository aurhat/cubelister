/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
 *   mimosius@users.sourceforge.net                                        *
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

#include "Csl.h"
#include "engine/CslEngine.h"
#include "engine/CslGameSauerbraten.h"
#include "engine/CslGameAssaultCube.h"
#include "engine/CslGameBloodFrontier.h"
#include "engine/CslGameCube.h"
#include "CslApp.h"
#include "CslGeoIP.h"
#include "CslDlgAbout.h"
#include "CslDlgAddMaster.h"
#include "CslDlgConnectWait.h"
#include "CslDlgGeneric.h"
#include "CslGameProcess.h"
#include "CslFrame.h"
#ifndef __WXMSW__
#include "csl_icon_png.h"
#endif
#ifndef __WXMAC__
#include "img/csl_24.xpm"
#endif
#include "img/master_24.xpm"
#include "img/close_14.xpm"
#include "img/close_high_14.xpm"
#include "img/close_press_14.xpm"

#define CSL_TIMER_SHOT 250

enum
{
    CSL_BUTTON_SEARCH_CLOSE = MENU_CUSTOM_END+1,
    CSL_TEXT_SEARCH,
    CSL_BUTTON_SEARCH,
    CSL_RADIO_SEARCH_SERVER,
    CSL_RADIO_SEARCH_PLAYER
};


BEGIN_EVENT_TABLE(CslFrame,wxFrame)
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
    EVT_ICONIZE(CslFrame::OnIconize)
    EVT_TASKBAR_LEFT_DOWN(CslFrame::OnTrayIcon)
    #endif
    EVT_SHOW(CslFrame::OnShow)
    EVT_CLOSE(CslFrame::OnClose)
    CSL_EVT_TOOLTIP(wxID_ANY,CslFrame::OnToolTip)
    CSL_EVT_VERSIONCHECK(wxID_ANY,CslFrame::OnVersionCheck)
    CSL_EVT_PROCESS(wxID_ANY,CslFrame::OnEndProcess)
    CSL_EVT_IPC(wxID_ANY,CslFrame::OnIPC)
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


CslFrame::CslFrame(wxWindow* parent,int id,const wxString& title,
                   const wxPoint& pos,const wxSize& size,long style):
        wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
    m_oldSelectedInfo=NULL;
#ifndef __WXMAC__
    m_tbIcon=NULL;
#endif

    m_engine=::wxGetApp().GetCslEngine();
    if (m_engine->Init(this,CSL_UPDATE_INTERVAL_MIN,1000/CSL_TIMER_SHOT))
    {
        m_engine->AddGame(new CslGameSauerbraten());
        m_engine->AddGame(new CslGameAssaultCube());
        m_engine->AddGame(new CslBloodFrontier());
        m_engine->AddGame(new CslGameCube());
    }

    LoadSettings();

    if (!LoadLocators())
    {
        CslGeoIP::AddService(wxT("GeoIPTool"),
                             wxT("http://geoiptool.com/"),
                             wxT("?IP="));
        CslGeoIP::AddService(wxT("MaxMind (Demo)"),
                             wxT("http://www.maxmind.com/"),
                             wxT("app/locate_ip?ips="));
    }

    CslTTS::Init(wxGetApp().GetLanguage());

    wxString user=::wxGetUserId()+wxT("|CSL");
    CslIrcNetwork *network;
    network=new CslIrcNetwork(wxT("GameSurge"),user,user+wxT("^1"));
    network->AddServer(new CslIrcServer(network,wxT("irc.gamesurge.net"),6667));
    network->AddAutoChannel(new CslIrcChannel(wxT("#cubelister")));
    g_CslIrcNetworks.Add(network);
    network=new CslIrcNetwork(wxT("Quakenet"),user,user+wxT("^1"));
    network->AddServer(new CslIrcServer(network,wxT("xs4all.nl.quakenet.org"),6667));
    network->AddAutoChannel(new CslIrcChannel(wxT("#cubelister")));
    g_CslIrcNetworks.Add(network);
    network=new CslIrcNetwork(wxT("Freenode"),user,user+wxT("^1"));
    network->AddServer(new CslIrcServer(network,wxT("irc.freenode.org"),6667));
    network->AddAutoChannel(new CslIrcChannel(wxT("#cubelister")));
    g_CslIrcNetworks.Add(network);

    m_engine->SetUpdateInterval(g_cslSettings->updateInterval);

    m_imgListTree.Create(24,24,true);
    m_imgListTree.Add(wxBitmap(master_24_xpm));

    m_imgListButton.Create(14,14,true);
    m_imgListButton.Add(wxBitmap(close_14_xpm));
    m_imgListButton.Add(wxBitmap(close_high_14_xpm));
    m_imgListButton.Add(wxBitmap(close_press_14_xpm));

    CslListCtrl::CreateCountryFlagImageList();

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
    list_ctrl_player_search->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
#endif

    tree_ctrl_games->SetImageList(&m_imgListTree);
    m_treeGamesRoot=tree_ctrl_games->AddRoot(wxEmptyString,-1,-1);

    if (m_engine->IsOk())
    {
        CslGame *game;
        const vector<CslGame*>& games=m_engine->GetGames();

        loopv(games)
        {
            game=games[i];

            CslMaster *master=new CslMaster(game->GetDefaultMasterConnection());
            if (game->AddMaster(master)<0)
                delete master;

            if (!serverLoaded)
            {
                //add default servers here
            }

            const char **icon=game->GetIcon(24);
            bool select=g_cslSettings->lastGame.IsEmpty() ? i==0 :
                        g_cslSettings->lastGame==game->GetName();
            m_imgListTree.Add(icon ? wxBitmap(icon):wxBitmap(24,24));
            TreeAddGame(game,icon ? i+1:-1,select);
        }

        CslMenu::EnableItem(MENU_ADD);

        m_ipcServer=new CslIpcServer(this);
        if (!m_ipcServer->Create(CSL_IPC_SERV))
        {
            LOG_DEBUG("couldn't create IPC server socket\n");
            delete m_ipcServer;
            m_ipcServer=NULL;
        }

        m_timerUpdate=g_cslSettings->updateInterval/CSL_TIMER_SHOT;
        m_timerCount=0;
        m_timerInit=true;
        m_timer.SetOwner(this);
        m_timer.Start(CSL_TIMER_SHOT);
    }
    else
    {
        wxMessageBox(_("Failed to initialise internal engine!\n"\
                       _L_"Please try to restart the application."),
                     _("Fatal error!"),wxICON_ERROR,this);
        m_engine->DeInit();
    }

    m_versionCheckThread=new CslVersionCheckThread(this);

    if (m_versionCheckThread->IsOk())
        m_versionCheckThread->Run();
}

CslFrame::~CslFrame()
{
    if (m_timer.IsRunning())
        m_timer.Stop();

    if (CslGameConnection::IsPlaying())
    {
        wxString error;
        CslGameConnection::GetInfo()->GetGame().GameEnd(error);
    }

    if (m_ipcServer)
    {
        delete m_ipcServer;
        m_ipcServer=NULL;
    }

    CslTTS::DeInit();

    if (m_engine->IsOk())
    {
        SaveSettings();
        SaveServers();
        m_engine->DeInit();
    }

    SaveLocators();

    CslToolTip::ResetTip();

    delete g_cslSettings;

#ifndef __WXMAC__
    if (m_tbIcon)
        delete m_tbIcon;
#endif

    if (m_versionCheckThread)
    {
        m_versionCheckThread->Delete();
        delete m_versionCheckThread;
    }

    m_AuiMgr.UnInit();
}

void CslFrame::CreateMainMenu()
{
    wxMenu *menu;
    wxMenuBar *menubar=new wxMenuBar();

    // Do not add the File menu on wxMAC, since Preferences and Exit are
    // getting moved to the "Mac menu" so the File menu is empty then.
    // Add Prefernces and Exit to any other menu.
#ifndef __WXMAC__
    menu=new wxMenu();
    menubar->Append(menu,_("&File"));
    CslMenu::AddItem(menu,wxID_PREFERENCES,_("&Settings"),wxART_SETTINGS);
    CslMenu::AddItem(menu,wxID_EXIT,_("&Exit"),wxART_QUIT);
#endif

    menuMaster=new wxMenu();
    CslMenu::AddItem(menuMaster,MENU_UPDATE,_("&Update from master\tF5"),wxART_RELOAD);
    menuMaster->AppendSeparator();
    CslMenu::AddItem(menuMaster,MENU_ADD,_("Add a &new master ..."),wxART_ADD_BOOKMARK);
    CslMenu::AddItem(menuMaster,MENU_DEL,_("&Remove master"),wxART_DEL_BOOKMARK);
    menuMaster->AppendSeparator();
    CslMenu::AddItem(menuMaster,MENU_GAME_SERVER_COUNTRY,_("&Servers by country"),wxART_COUNTRY_UNKNOWN);
    CslMenu::AddItem(menuMaster,MENU_GAME_PLAYER_COUNTRY,_("&Players by country"),wxART_COUNTRY_UNKNOWN);
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
    CslMenu::AddItem(menu,MENU_VIEW_PLAYER_SEARCH,_("Show player search result"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_COUNTRY,_("Show country list"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_FAVOURITES,_("Show favourites"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_USER_CHAT,_("Show user chat\tCTRL+U"),
                     wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(menu,MENU_VIEW_SEARCH,_("Show search bar\tCTRL+F"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_OUTPUT,_("Show game output\tCTRL+O"),
                     wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItem(menu,MENU_VIEW_TRAFFIC,_("&Traffic statistics"),wxART_NONE);
    menu->AppendSeparator();
    CslMenu::AddItem(menu,MENU_VIEW_AUTO_SORT,_("Sort lists automatically\tCTRL+L"),wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(menu,MENU_VIEW_RELAYOUT,_("Reset layout"),wxART_NONE);
    menubar->Append(menu,_("&View"));

#ifndef __WXMAC__
    menu=new wxMenu();
    CslMenu::AddItem(menu,wxID_ABOUT,_("A&bout"),wxART_ABOUT);
    menubar->Append(menu,_("&Help"));
#endif

    SetMenuBar(menubar);

    CslMenu::SetMainMenu(menubar);

    CslMenu::EnableItem(MENU_ADD,false);
    CslMenu::EnableItem(MENU_DEL,false);
    CslMenu::EnableItem(MENU_UPDATE,false);
    CslMenu::EnableItem(MENU_VIEW_SEARCH,g_cslSettings->showSearch);
    CslMenu::EnableItem(MENU_VIEW_AUTO_SORT,g_cslSettings->autoSortColumns);
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
    list_ctrl_player_search=new CslListCtrlPlayerSearch(pane_main,wxID_ANY,wxDefaultPosition,wxDefaultSize,listStyle);
    pane_country=new CslPanelCountry(pane_main,listStyle);
    list_ctrl_master=new CslListCtrlServer(pane_main,CslListCtrlServer::CSL_LIST_MASTER,
                                           wxDefaultPosition,wxDefaultSize,listStyle);
    notebook_irc=new CslIrcNotebook(this);
    list_ctrl_favourites=new CslListCtrlServer(pane_main,CslListCtrlServer::CSL_LIST_FAVOURITE,
            wxDefaultPosition,wxDefaultSize,listStyle);
    pane_search=new wxPanel(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxNO_BORDER|wxTAB_TRAVERSAL);
    text_ctrl_search=new wxTextCtrl(pane_search,CSL_TEXT_SEARCH,wxEmptyString,wxDefaultPosition,
                                    wxDefaultSize,wxTE_PROCESS_ENTER|wxTE_RICH|wxTE_RICH2);
    text_search_result=new wxStaticText(pane_search,wxID_ANY,wxEmptyString);
    button_search=new wxButton(pane_search,CSL_BUTTON_SEARCH,_("&Search"));
    button_search_clear=new wxBitmapButton(pane_search,CSL_BUTTON_SEARCH_CLOSE,wxNullBitmap,
                                           wxDefaultPosition,wxDefaultSize,wxNO_BORDER);
    gauge_search=new wxGauge(pane_search,wxID_ANY,0,wxDefaultPosition,wxSize(button_search->GetBestSize().x,16));
    radio_search_server=new wxRadioButton(pane_search,CSL_RADIO_SEARCH_SERVER,_("Se&rvers"),
                                          wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
    radio_search_player=new wxRadioButton(pane_search,CSL_RADIO_SEARCH_PLAYER,_("&Players"));
    player_info=new CslPanelPlayer(pane_main,listStyle);
    m_playerInfos.add(player_info);

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

    radio_search_server->SetValue(true);
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

    player_info->ListCtrl()->ListInit(CslListCtrlPlayer::SIZE_MINI);

    list_ctrl_master->ListInit(list_ctrl_favourites);
    list_ctrl_favourites->ListInit(list_ctrl_master);

    notebook_irc->AddPage(new CslIrcPanel(notebook_irc),_("Chat"),true);

    // see wx_wxbitmap.html
#ifdef __WXMSW__
    SetIcon(wxICON(appicon));
#else
    wxMemoryInputStream stream(csl_icon_png,sizeof(csl_icon_png));
    wxImage image(stream,wxBITMAP_TYPE_PNG);
    wxBitmap bitmap(image);
    wxIcon icon;
    icon.CopyFromBitmap(bitmap);
    SetIcon(icon);
#endif

#ifndef __WXMAC__
    ToggleTrayIcon();
#endif
}

void CslFrame::DoLayout()
{
    wxInt32 i=0;

    sizer_main=new wxFlexGridSizer(2,1,0,0);
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

#define CSL_CAPTION_GAMES _("Games")
#define CSL_CAPTION_MASTER_LIST_SERVERS _("Master list servers")
#define CSL_CAPTION_FAVOURITE_LIST_SERVERS _("Favourite servers")
#define CSL_CAPTION_PLAYERS_SELECTED _("Selected server")
#define CSL_CAPTION_PLAYER_SEARCH _("Player search result")
#define CSL_CAPTION_SERVER_COUNTRY _("Server countries")
#define CSL_CAPTION_PLAYER_COUNTRY _("Player countries")
#define CSL_CAPTION_SERVER_INFO _("Server info")
#define CSL_CAPTION_IRC _("IRC chat")

    wxSize size=list_ctrl_master->GetBestSize();

    m_AuiMgr.AddPane(list_ctrl_master,wxAuiPaneInfo().Name(wxT("masterlist")).
                     CloseButton(false).Center().BestSize(size).MinSize(size.x,20));
    m_AuiMgr.AddPane(list_ctrl_favourites,wxAuiPaneInfo().Name(wxT("favlist")).
                     Bottom().Row(2).BestSize(size).MinSize(size).FloatingSize(600,240));
    m_AuiMgr.AddPane(tree_ctrl_games,wxAuiPaneInfo().Name(wxT("games")).
                     Caption(_("Games")).Left().Layer(3).Position(0).
                     MinSize(240,20).BestSize(240,250).FloatingSize(240,250));
    m_AuiMgr.AddPane(list_ctrl_info,wxAuiPaneInfo().Name(wxT("info")).
                     Left().Layer(3).Position(1).
                     MinSize(240,20).BestSize(360,200).FloatingSize(360,200));
    m_AuiMgr.AddPane(player_info,wxAuiPaneInfo().Name(wxT("players0")).
                     Left().Layer(3).Position(2).
                     MinSize(240,20).BestSize(CslListCtrlPlayer::BestSizeMini).FloatingSize(280,200));
    m_AuiMgr.AddPane(list_ctrl_player_search,wxAuiPaneInfo().Name(wxT("search")).
                     Left().Layer(2).Position(0).
                     MinSize(240,80).BestSize(240,150).FloatingSize(240,150));
    m_AuiMgr.AddPane(pane_country,wxAuiPaneInfo().Name(wxT("country")).
                     Left().Layer(2).Position(1).
                     MinSize(240,80).BestSize(240,150).FloatingSize(240,150));
    m_AuiMgr.AddPane(notebook_irc,wxAuiPaneInfo().Name(wxT("irc")).
                     Left().Layer(0).Position(0).Row(3).Direction(3).
                     MinSize(240,20).BestSize(360,200).FloatingSize(360,200));

    m_defaultLayout=m_AuiMgr.SavePerspective();

    if (!g_cslSettings->layout.IsEmpty())
        m_AuiMgr.LoadPerspective(g_cslSettings->layout,false);

    wxAuiPaneInfo *pane;

    pane=&m_AuiMgr.GetPane(tree_ctrl_games);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_GAMES);
        CslMenu::CheckItem(MENU_VIEW_GAMES,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(list_ctrl_info);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_SERVER_INFO);
        CslMenu::CheckItem(MENU_VIEW_SERVER_INFO,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(list_ctrl_master);
    if (pane->IsOk())
        pane->Caption(CSL_CAPTION_MASTER_LIST_SERVERS);
    pane=&m_AuiMgr.GetPane(list_ctrl_favourites);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_FAVOURITE_LIST_SERVERS);
        CslMenu::CheckItem(MENU_VIEW_FAVOURITES,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(player_info);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYERS_SELECTED);
        CslMenu::CheckItem(MENU_VIEW_PLAYER_LIST,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(list_ctrl_player_search);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYER_SEARCH);
        CslMenu::CheckItem(MENU_VIEW_PLAYER_SEARCH,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(pane_country);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYER_COUNTRY);
        CslMenu::CheckItem(MENU_VIEW_COUNTRY,pane->IsShown());
    }
    pane=&m_AuiMgr.GetPane(notebook_irc);
    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_IRC);
        CslMenu::CheckItem(MENU_VIEW_USER_CHAT,pane->IsShown());
    }

    SetListCaption(CslListCtrlServer::CSL_LIST_MASTER);
    SetListCaption(CslListCtrlServer::CSL_LIST_FAVOURITE);

    m_AuiMgr.Update();

    sizer_main->Fit(this);
    Layout();

    if (g_cslSettings->frameSizeMax!=wxDefaultSize)
    {
        m_maximised=true;
        SetMinSize(g_cslSettings->frameSize);
#ifdef __WXGTK__
        SetSize(g_cslSettings->frameSizeMax);
#endif
        Maximize(true);
    }
    else
    {
        m_maximised=false;
        SetMinSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT));
        SetSize(g_cslSettings->frameSize);
    }

#ifdef __WXMSW__
    CentreOnScreen();
#endif

    //connect after calling size functions, so g_cslSettings->frameSize has the right value
    Connect(wxEVT_SIZE,wxSizeEventHandler(CslFrame::OnSize),NULL,this);
}

void CslFrame::PanelCountrySetCaption(CslServerInfo *info)
{
    wxString caption;
    wxUint32 mode=pane_country->GetMode();

    caption=mode==CslPanelCountry::MODE_SERVER ?
            CSL_CAPTION_SERVER_COUNTRY :
            CSL_CAPTION_PLAYER_COUNTRY;

    caption+=wxT(": ");

    if (!info || mode==CslPanelCountry::MODE_SERVER)
    {
        CslGame *game=TreeGetSelectedGame();
        CslMaster *master=TreeGetSelectedMaster();

        if (!game)
            return;

        if (master)
            caption+=master->GetConnection().GetAddress()+wxT(" (")+game->GetName()+wxT(")");
        else
            caption+=game->GetName();
    }
    else
        caption+=info->GetBestDescription()+wxT(" (")+info->GetGame().GetName()+wxT(")");

    wxAuiPaneInfo& pane=m_AuiMgr.GetPane(pane_country);
    if (pane.IsOk())
    {
        pane.Caption(caption);
        m_AuiMgr.Update();
    }
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

    wxSize size;

    if (view==CslListCtrlPlayer::SIZE_MICRO)
        size=CslListCtrlPlayer::BestSizeMicro;
    else
        size=CslListCtrlPlayer::BestSizeMini;

    wxAuiPaneInfo pane;

    pane.BestSize(size).DestroyOnClose().Caption(PlayerListGetCaption(info,false));

    if (name.IsEmpty())
    {
        long l;
        wxInt32 j,id=1;
        vector<wxInt32> ids;

        loopv(m_playerInfos)
        {
            if (i==0)
                continue;

            wxAuiPaneInfo &pane=m_AuiMgr.GetPane(m_playerInfos[i]);
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

    CslPanelPlayer *playerinfo=new CslPanelPlayer(pane_main,style);
    playerinfo->ListCtrl()->ListInit(view);
    playerinfo->ServerInfo(info);
    m_playerInfos.add(playerinfo);

    m_AuiMgr.AddPane(playerinfo,pane);
    m_AuiMgr.Update();

    pane_main->Thaw();

    info->PingExt(true);
    m_engine->PingEx(info,true);
}

void CslFrame::TogglePane(wxInt32 id,bool forceShow)
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
        case MENU_VIEW_USER_CHAT:
            pane=&m_AuiMgr.GetPane(notebook_irc);
            break;
        case MENU_VIEW_PLAYER_LIST:
            pane=&m_AuiMgr.GetPane(player_info);
            break;
        case MENU_VIEW_PLAYER_SEARCH:
            pane=&m_AuiMgr.GetPane(list_ctrl_player_search);
            break;
        case MENU_VIEW_COUNTRY:
            pane=&m_AuiMgr.GetPane(pane_country);
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

    if (shown && forceShow)
        return;

    pane->Show(!shown);
    CslMenu::CheckItem(id,!shown);

    m_AuiMgr.Update();
}

void CslFrame::ToggleShow()
{
    if (!IsShown())
    {
#ifdef __WXGTK__
        // neccessary otherwise the window hasn't the right frame size after
        // minimising from maximised state and then restoring the last frame size
        SetSize(g_cslSettings->frameSize);
        Show();
        Raise();
#endif
#ifdef __WXMSW__
        Show();
        // neccessary otherwise the window doesn't get raised after
        // minimising using the minimise button or context menu function
        Maximize(m_maximised);
        Raise();
#endif
    }
    else
    {
        if (!::wxGetApp().IsActive())
            Raise();
        else
            Hide();
    }
}

#ifndef __WXMAC__
void CslFrame::ToggleTrayIcon()
{
    if (g_cslSettings->systray&CSL_USE_SYSTRAY && !m_tbIcon)
    {
        m_tbIcon=new wxTaskBarIcon();
        m_tbIcon->SetNextHandler(this);
        m_tbIcon->SetIcon(csl_24_xpm);
    }
    else if (m_tbIcon && !(g_cslSettings->systray&CSL_USE_SYSTRAY))
    {
        if (!IsShown())
            Show();
        delete m_tbIcon;
        m_tbIcon=NULL;
    }
}
#endif

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
        caption<<wxT("  (")<<_("Filters:")<<wxT(" ");
        if (filter&CSL_FILTER_OFFLINE)
            caption<<MENU_SERVER_FILTER_OFF_STR<<(wxT(", "));
        if (filter&CSL_FILTER_FULL)
            caption<<MENU_SERVER_FILTER_FULL_STR<<(wxT(", "));
        if (filter&CSL_FILTER_EMPTY)
            caption<<MENU_SERVER_FILTER_EMPTY_STR<<(wxT(", "));
        if (filter&CSL_FILTER_NONEMPTY)
            caption<<MENU_SERVER_FILTER_NONEMPTY_STR<<(wxT(", "));
        if (filter&CSL_FILTER_MM2)
            caption<<MENU_SERVER_FILTER_MM2_STR<<(wxT(", "));
        if (filter&CSL_FILTER_MM3)
            caption<<MENU_SERVER_FILTER_MM3_STR<<(wxT(", "));
        caption.Remove(caption.Length()-2);
        caption<<wxT(")");
    }

    caption<<wxT(" ")<<addon;

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

    m_timer.Stop();

    CslMenu::EnableItem(MENU_UPDATE,false);
    tree_ctrl_games->Enable(false);

    CslGame *game=master->GetGame();
    if (game)
    {
        CslServerInfo *info;
        wxUint32 now=wxDateTime::Now().GetTicks();
        vector<CslServerInfo*>& servers=game->GetServers();

        loopvrev(servers)
        {
            info=servers[i];

            if (g_cslSettings->cleanupServers &&
                now-info->LastSeen>g_cslSettings->cleanupServers &&
                !(g_cslSettings->cleanupServersKeepFav && info->IsFavourite()) &&
                !(g_cslSettings->cleanupServersKeepStats && info->HasStats()))
            {
                loopvj(m_playerInfos)
                {
                    if (j!=0 && m_playerInfos[j]->ServerInfo()==info)
                    {
                        info=NULL;
                        break;
                    }
                }

                if (info)
                {
                    LOG_DEBUG("Cleanup: %s - %s (%s)\n",U2A(game->GetName()),
                              U2A(info->GetBestDescription()),U2A(info->Addr.IPAddress()));

                    if (m_playerInfos.length() && m_playerInfos[0]->ServerInfo()==info)
                    {
                        m_playerInfos[0]->ServerInfo(NULL);
                        m_playerInfos[0]->UpdateData();
                        wxAuiPaneInfo& pane=m_AuiMgr.GetPane(m_playerInfos[0]);
                        if (pane.IsOk())
                        {
                            pane.Caption(PlayerListGetCaption(NULL,true));
                            m_AuiMgr.Update();
                        }
                    }

                    if (info->IsFavourite())
                        list_ctrl_favourites->RemoveServer(NULL,info,-1);

                    game->DeleteServer(info);
                }
            }
        }
    }

    SetStatusText(_("Sending request to master: ")+master->GetConnection().GetAddress(),1);
    wxInt32 num=m_engine->UpdateFromMaster(master);
    if (num<0)
        SetStatusText(_("Error on update from master: ")+master->GetConnection().GetAddress(),1);
    else
    {
        m_timerCount=0;
        list_ctrl_master->ListClear();
        SetStatusText(wxString::Format(_("Got %d servers from master"),num),1);
    }

    CslMenu::EnableItem(MENU_UPDATE);
    tree_ctrl_games->Enable();

    m_timerInit=true;
    m_timer.Start(CSL_TIMER_SHOT);
}

void CslFrame::ConnectToServer(CslServerInfo *info,wxInt32 pass)
{
    if (info)
    {
        CslServerInfo *old=CslGameConnection::GetInfo();

        if (old && CslGameConnection::IsWaiting())
        {
            list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,old);
            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,old);
        }

        if (!CslGameConnection::Prepare(info,pass))
            return;
    }
    else if (!(info=CslGameConnection::GetInfo()))
        return;

    if (!CslGameConnection::Connect())
        return;

    list_ctrl_info->UpdateInfo(info);
    list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,true,false,info);
    list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,true,false,info);
}

void CslFrame::LoadSettings()
{
    g_cslSettings=new CslSettings;

    if (!::wxFileExists(CSL_SETTINGS_FILE))
        return;

    long int val;
#if 0 // enable later if there is any custom column size support
    double dval;
#endif
    wxUint32 version;
    wxString s;

    wxFileConfig config(wxT(""),wxT(""),CSL_SETTINGS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);

    config.SetPath(wxT("/Version"));
    config.Read(wxT("Version"),&val,0); version=val;

    config.SetPath(wxT("/Gui"));
    if (config.Read(wxT("SizeX"),&val)) g_cslSettings->frameSize.SetWidth(val);
    if (config.Read(wxT("SizeY"),&val)) g_cslSettings->frameSize.SetHeight(val);
    if (config.Read(wxT("SizeMaxX"),&val)) g_cslSettings->frameSizeMax.SetWidth(val);
    if (config.Read(wxT("SizeMaxY"),&val)) g_cslSettings->frameSizeMax.SetHeight(val);
    if (config.Read(wxT("Layout"),&s)) g_cslSettings->layout=s;
    if (config.Read(wxT("Systray"),&val)) g_cslSettings->systray=val;
    if (config.Read(wxT("TTS"),&val)) g_cslSettings->tts=val!=0;
    if (config.Read(wxT("TTSVolume"),&val))
    {
        if (val>0 && val<=100)
            g_cslSettings->ttsVolume=val;
    }
    if (config.Read(wxT("UpdateInterval"),&val))
    {
        if (val>=CSL_UPDATE_INTERVAL_MIN && val<=CSL_UPDATE_INTERVAL_MAX)
            g_cslSettings->updateInterval=val;
    }
    if (config.Read(wxT("NoUpdatePlaying"),&val)) g_cslSettings->dontUpdatePlaying=val!=0;
    if (config.Read(wxT("ShowSearch"),&val)) g_cslSettings->showSearch=val!=0;
    if (config.Read(wxT("FilterMaster"),&val)) g_cslSettings->filterMaster=val;
    if (config.Read(wxT("FilterFavourites"),&val)) g_cslSettings->filterFavourites=val;
    if (config.Read(wxT("WaitOnFullServer"),&val))
    {
        if (val>=CSL_WAIT_SERVER_FULL_MIN && val<=CSL_WAIT_SERVER_FULL_MAX)
            g_cslSettings->waitServerFull=val;
    }
    if (config.Read(wxT("CleanupServers"),&val))
    {
        if ((!val || (val && val>=86400)) && val<=CSL_CLEANUP_SERVERS_MAX*86400)
            g_cslSettings->cleanupServers=val;
    }
    if (config.Read(wxT("CleanupServersKeepFavourites"),&val))
        g_cslSettings->cleanupServersKeepFav=val!=0;
    if (config.Read(wxT("CleanupServersKeepStats"),&val))
        g_cslSettings->cleanupServersKeepStats=val!=0;
    if (config.Read(wxT("TooltipDelay"),&val))
    {
        if (val>=CSL_TOOLTIP_DELAY_MIN && val<=CSL_TOOLTIP_DELAY_MAX)
            g_cslSettings->tooltipDelay=val/CSL_TOOLTIP_DELAY_STEP*CSL_TOOLTIP_DELAY_STEP;
    }
    if (config.Read(wxT("PingGood"),&val))
    {
        if (val<=9999)
            g_cslSettings->pinggood=val;
    }
    if (config.Read(wxT("PingBad"),&val))
    {
        if (val<g_cslSettings->pinggood)
            g_cslSettings->pingbad=g_cslSettings->pinggood;
        else
            g_cslSettings->pingbad=val;
    }
    if (config.Read(wxT("GameOutputPath"),&s)) g_cslSettings->gameOutputPath=s;
    if (g_cslSettings->gameOutputPath.IsEmpty())
        g_cslSettings->gameOutputPath=wxStandardPaths().GetUserDataDir();
    if (config.Read(wxT("AutoSaveOutput"),&val)) g_cslSettings->autoSaveOutput=val!=0;
    if (config.Read(wxT("LastSelectedGame"),&s)) g_cslSettings->lastGame=s;
    if (config.Read(wxT("MinPlaytime"),&val))
    {
        if (val>=CSL_MIN_PLAYTIME_MIN && val<=CSL_MIN_PLAYTIME_MAX)
            g_cslSettings->minPlaytime=val;
    }

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    if (config.Read(wxT("AutoSort"),&val)) g_cslSettings->autoSortColumns=val!=0;
#if 0 // enable later if there is any custom column size support
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
#endif
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
        if (config.Read(wxT("PreScript"),&s)) settings.PreScript=s;
        if (config.Read(wxT("PostScript"),&s)) settings.PostScript=s;
        if (config.Read(wxT("ExpertConfig"),&val)) settings.Expert=val!=0;
        if (!settings.Binary.IsEmpty() && !::wxFileExists(settings.Binary))
            settings.Binary=wxEmptyString;
        if (!settings.GamePath.IsEmpty() && !::wxDirExists(settings.GamePath))
            settings.GamePath=wxEmptyString;
    }
}

void CslFrame::SaveSettings()
{
    wxString dir=::wxPathOnly(CSL_SETTINGS_FILE);

    if (!::wxDirExists(dir))
        ::wxMkdir(dir,0700);

    wxFileConfig config(wxT(""),wxT(""),CSL_SETTINGS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);
    config.SetUmask(0077);
    config.DeleteAll();

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"),CSL_CONFIG_VERSION);

    /* GUI */
    wxSize size=g_cslSettings->frameSize;
    config.SetPath(wxT("/Gui"));
    config.Write(wxT("SizeX"),(long int)size.GetWidth());
    config.Write(wxT("SizeY"),(long int)size.GetHeight());
    size=m_maximised ? GetSize() : wxDefaultSize;
    config.Write(wxT("SizeMaxX"),(long int)size.GetWidth());
    config.Write(wxT("SizeMaxY"),(long int)size.GetHeight());
    config.Write(wxT("Layout"),m_AuiMgr.SavePerspective());
    config.Write(wxT("Systray"),g_cslSettings->systray);
    config.Write(wxT("TTS"),g_cslSettings->tts);
    config.Write(wxT("TTSVolume"),g_cslSettings->ttsVolume);
    config.Write(wxT("UpdateInterval"),(long int)g_cslSettings->updateInterval);
    config.Write(wxT("NoUpdatePlaying"),g_cslSettings->dontUpdatePlaying);
    config.Write(wxT("ShowSearch"),g_cslSettings->showSearch);
    config.Write(wxT("FilterMaster"),(long int)g_cslSettings->filterMaster);
    config.Write(wxT("FilterFavourites"),(long int)g_cslSettings->filterFavourites);
    config.Write(wxT("WaitOnFullServer"),(long int)g_cslSettings->waitServerFull);
    config.Write(wxT("CleanupServers"),(long int)g_cslSettings->cleanupServers);
    config.Write(wxT("CleanupServersKeepFavourites"),(long int)g_cslSettings->cleanupServersKeepFav);
    config.Write(wxT("CleanupServersKeepStats"),(long int)g_cslSettings->cleanupServersKeepStats);
    config.Write(wxT("TooltipDelay"),(long int)g_cslSettings->tooltipDelay);
    config.Write(wxT("PingGood"),(long int)g_cslSettings->pinggood);
    config.Write(wxT("PingBad"),(long int)g_cslSettings->pingbad);
    config.Write(wxT("GameOutputPath"),g_cslSettings->gameOutputPath);
    config.Write(wxT("AutoSaveOutput"),g_cslSettings->autoSaveOutput);
    config.Write(wxT("LastSelectedGame"),g_cslSettings->lastGame);
    config.Write(wxT("MinPlaytime"),(long int)g_cslSettings->minPlaytime);

    /* ListCtrl */
    config.SetPath(wxT("/List"));
#if 0 // enable later if there is any custom column size support
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
#endif
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
        config.Write(wxT("PreScript"),settings.PreScript);
        config.Write(wxT("PostScript"),settings.PostScript);
        config.Write(wxT("Options"),settings.Options);
        config.Write(wxT("ExpertConfig"),settings.Expert);
    }
}

bool CslFrame::LoadServers(wxUint32 *numm,wxUint32 *nums)
{
    if (!::wxFileExists(CSL_SERVERS_FILE))
        return false;

    long int val;
    bool read_server;
    wxUint32 mc=0,sc=0,tmc=0,tsc=0;
    CslMaster *master;
    vector<CslIDMapping*> mappings;
    vector<wxInt32> ids;

    wxString addr,path,pass1,pass2,description;
    wxUint16 port=0,iport=0;
    wxUint32 view=0;
    wxUint32 events=0;
    wxUint32 lastSeen=0;
    wxUint32 playLast=0;
    wxUint32 playTimeLastGame=0;
    wxUint32 playTimeTotal=0;
    wxUint32 connectedTimes=0;
    wxInt32 gt;

    wxFileConfig config(wxT(""),wxT(""),CSL_SERVERS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);

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
                config.Read(wxT("Port"),&val,games[gt]->GetDefaultGamePort()); port=val;
                // use 0 as default infoport to import older servers.ini and
                // set the appropriate infoport when creating the CslServerInfo
                config.Read(wxT("InfoPort"),&val,0); iport=val;
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
                    config.Read(wxT("Events"),&val,0);
                    events=(wxUint32)val;
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
                CslServerInfo *info=new CslServerInfo(games[gt],addr,port,iport ? iport:port+1,
                                                      view,lastSeen,playLast,playTimeLastGame,
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
                    long type;
                    wxInt32 pos,j=0;
                    wxString guiview;

                    m_engine->ResolveHost(info);
                    info->RegisterEvents(events);

                    while (config.Read(wxString::Format(wxT("GuiView%d"),j++),&guiview))
                    {
                        if ((pos=guiview.Find(wxT(':')))<0)
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
    wxString s=::wxPathOnly(CSL_SERVERS_FILE);

    if (!::wxDirExists(s))
        if (!::wxMkdir(s,0700))
            return;

    wxFileConfig config(wxT(""),wxT(""),CSL_SERVERS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);
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
            config.Write(wxT("Port"),info->GamePort);
            config.Write(wxT("InfoPort"),info->InfoPort);
            config.Write(wxT("Password"),info->Password);
            config.Write(wxT("AdminPassword"),info->PasswordAdmin);
            config.Write(wxT("Description"),info->DescriptionOld);
            const vector<wxInt32> masters=info->GetMasterIDs();
            loopvk(masters) config.Write(wxString::Format(wxT("Master%d"),k),masters[k]);
            config.Write(wxT("View"),(int)info->View);
            config.Write(wxT("Events"),(int)info->GetRegisteredEvents());
            config.Write(wxT("LastSeen"),(int)info->LastSeen);
            config.Write(wxT("PlayLast"),(int)info->PlayedLast);
            config.Write(wxT("PlayTimeLastGame"),(int)info->PlayTimeLast);
            config.Write(wxT("PlayTimeTotal"),(int)info->PlayTimeTotal);
            config.Write(wxT("ConnectedTimes"),(int)info->ConnectedTimes);

            wxUint32 l=0;
            loopvk(m_playerInfos)
            {
                if (k==0)
                    continue;
                CslPanelPlayer *list=m_playerInfos[k];
                if (list->ServerInfo()==info)
                {
                    wxAuiPaneInfo& pane=m_AuiMgr.GetPane(list);
                    if (!pane.IsOk())
                        continue;

                    wxString view=wxString::Format(wxT("%d:"),list->ListCtrl()->View())+pane.name;
                    config.Write(wxString::Format(wxT("GuiView%d"),l++),view);
                }
            }
        }
    }
}

wxUint32 CslFrame::LoadLocators()
{
    if (!::wxFileExists(CSL_LOCATORS_FILE))
        return 0;

    long int val;
    wxUint32 version,i=0;
    wxString Name,Host,Path;

    wxFileConfig config(wxT(""),wxT(""),CSL_LOCATORS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);

    config.SetPath(wxT("/Version"));
    config.Read(wxT("Version"),&val,0); version=val;

    while (true)
    {
        config.SetPath(wxString::Format(wxT("/%d"),i++));

        if (config.Read(wxT("Name"),&Name) &&
            config.Read(wxT("Host"),&Host) &&
            config.Read(wxT("Path"),&Path))
            CslGeoIP::AddService(Name,Host,Path);
        else
            break;
    }

    return i;
}

void CslFrame::SaveLocators()
{
    wxString s=::wxPathOnly(CSL_LOCATORS_FILE);

    if (!::wxDirExists(s))
        if (!::wxMkdir(s,0700))
            return;

    wxFileConfig config(wxT(""),wxT(""),CSL_LOCATORS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);
    config.SetUmask(0077);
    config.DeleteAll();

    wxUint32 i;
    CslGeoIPService *service;
    const CslGeoIPServices& services=CslGeoIP::GetServices();

    if (services.IsEmpty())
        return;

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"),CSL_LOCATORCONFIG_VERSION);

    for (i=0;i<services.GetCount();i++)
    {
        service=services.Item(i);

        config.SetPath(wxString::Format(wxT("/%d"),i));
        config.Write(wxT("Name"),service->Name);
        config.Write(wxT("Host"),service->Host);
        config.Write(wxT("Path"),service->Path);
    }
}

void CslFrame::OnPong(wxCommandEvent& event)
{
    CslPongPacket *packet=(CslPongPacket*)event.GetClientData();

    if (!packet)
        return;

    if (packet->Type==CSL_PONG_TYPE_PING)
    {
        if (packet->Info->HasEvents())
        {
#define TT_SPACER (s.IsEmpty() ? wxT(" "):wxT(", "))
            wxString s;

            if (packet->Info->HasEvent(CslServerEvents::EVENT_EMPTY))
                s<<wxT(" ")<<wxT("empty");
            if (packet->Info->HasEvent(CslServerEvents::EVENT_NOT_EMPTY))
                s<<TT_SPACER<<wxT("not empty anymore");

            if (packet->Info->HasEvent(CslServerEvents::EVENT_FULL))
                s<<TT_SPACER<<wxT("full");
            if (packet->Info->HasEvent(CslServerEvents::EVENT_NOT_FULL))
                s<<TT_SPACER<<wxT("not full anymore");

            if (packet->Info->HasEvent(CslServerEvents::EVENT_LOCKED))
                s<<TT_SPACER<<wxT("in locked mode");
            if (packet->Info->HasEvent(CslServerEvents::EVENT_PRIVATE))
                s<<TT_SPACER<<wxT("in private mode");

            if (!s.IsEmpty())
            {
                wxInt32 i=s.Find(wxT(','),true);

                m_toolTipTextRight=wxT("is now");

                if (i==wxNOT_FOUND)
                    m_toolTipTextRight<<s;
                else
                    m_toolTipTextRight<<s.Left(i)<<wxT(" ")<<wxT("and")<<s.Mid(i+1);

                m_toolTipTextRight<<wxT(".");
                m_toolTipTextLeft=packet->Info->GetBestDescription();
                m_toolTipTitle=wxT("CSL server notification");

                if (!CslGameConnection::IsPlaying())
                    CslToolTip::InitTip(this,true);

                CslTTS::Say(m_toolTipTitle+wxT(". ")+m_toolTipTextLeft+wxT(" ")+m_toolTipTextRight);
            }
#undef TT_SPACER
        }
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
        if (pane_country->GetMode()==CslPanelCountry::MODE_PLAYER_SINGLE)
        {
            if (packet->Info==m_oldSelectedInfo)
            {
                pane_country->Reset(CslPanelCountry::MODE_PLAYER_SINGLE);
                pane_country->UpdateData(packet->Info);
            }
        }
        else if (pane_country->GetMode()==CslPanelCountry::MODE_PLAYER_MULTI)
        {
            loopv(m_countryServers)
            {
                if (m_countryServers[i]!=packet->Info)
                    continue;

                if (packet->Info->Search)
                {
                    packet->Info->Search=false;
                    packet->Info->PingExt(false);
                    pane_country->UpdateData(packet->Info);
                }

                m_countryServers.remove(i);

                break;
            }
        }

        loopv(m_playerInfos)
        {
            if (m_playerInfos[i]->ServerInfo()==packet->Info)
                m_playerInfos[i]->UpdateData();
        }

        if (m_searchedServers.length() && radio_search_player->GetValue())
        {
            wxInt32 progress;
            bool found=false;

            loopv(m_searchedServers)
            {
                if (m_searchedServers[i]!=packet->Info)
                    continue;
                if (packet->Info->Search)
                {
                    packet->Info->Search=false;

                    progress=gauge_search->GetValue()+1;
                    gauge_search->SetValue(progress);

                    CslPlayerStats& stats=m_searchedServers[i]->PlayerStats;
                    loopvj(stats.m_stats)
                    {
                        CslPlayerStatsData *data=stats.m_stats[j];
                        if (data->Ok && data->Name.Lower().Contains(m_searchString))
                        {
                            list_ctrl_player_search->AddResult(packet->Info,data);
                            if (++m_searchResultPlayer==1)
                                TogglePane(MENU_VIEW_PLAYER_SEARCH,true);
                            found=true;
                        }
                    }

                    if (progress==m_searchedServers.length())
                    {
                        loopv(m_searchedServers) m_searchedServers[i]->PingExt(false);
                        m_searchedServers.setsize(0);
                    }

                    if (found)
                    {
                        list_ctrl_master->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER,true,true,packet->Info);
                        list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER,true,true,packet->Info);
                        m_searchResultServer++;
                    }
                    else
                    {
                        list_ctrl_master->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,false,false,packet->Info);
                        list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,false,false,packet->Info);
                    }
                }

                break;
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

    bool playing=CslGameConnection::IsPlaying();
    bool waiting=CslGameConnection::IsWaiting();

    if (!(g_cslSettings->dontUpdatePlaying && playing))
    {
        CslGame *game=TreeGetSelectedGame();
        CslMaster *master=TreeGetSelectedMaster();

        if (m_timerCount%m_timerUpdate==0)
        {
            if (game)
            {
                list_ctrl_master->ListUpdate(master? master->GetServers():game->GetServers());

                vector<CslServerInfo*> servers;
                m_engine->GetFavourites(servers);
                list_ctrl_favourites->ListUpdate(servers);

                wxTreeItemId item=tree_ctrl_games->GetSelection();
                if (item.IsOk())
                {
                    wxString s=master ? master->GetConnection().GetAddress() : game->GetName();
                    s+=s.Format(wxT(" (%d Players)"),list_ctrl_master->GetPlayerCount());
                    tree_ctrl_games->SetItemText(item,s);
                }
            }

            loopv(m_playerInfos)
            {
                m_playerInfos[i]->CheckServerStatus();
            }
        }

        m_timerCount++;

        bool green=game && m_engine->PingServers(game,m_timerInit);
        green|=m_engine->PingServersEx()!=0;

        if (green && !waiting && !playing)
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

    if (playing && m_timerCount%2==0)
    {
        CslGameProcess::ProcessOutput(CslGameProcess::INPUT_STREAM);
        CslGameProcess::ProcessOutput(CslGameProcess::ERROR_STREAM);

        CslStatusBar::Light(LIGHT_YELLOW);
    }
    else if (waiting)
    {
        ConnectToServer();

        if (CslGameConnection::IsWaiting() && m_timerCount%4==0)
        {
            CslServerInfo *info=CslGameConnection::GetInfo();

            if (!CslGameConnection::CountDown())
            {
                list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
                list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
            }
            else
                CslStatusBar::Light(LIGHT_RED);
        }
    }

    if (CslStatusBar::Light()!=LIGHT_GREY)
    {
        if ((lightCount=++lightCount%4)==0)
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
        m_oldSelectedInfo->PingExt(false);
    m_oldSelectedInfo=info;

    pane_country->Reset(CslPanelCountry::MODE_PLAYER_SINGLE);
    PanelCountrySetCaption(info);

    player_info->ServerInfo(info);

    info->PingExt(true);

    if (!m_engine->PingEx(info))
    {
        player_info->UpdateData();
        pane_country->UpdateData(info);
    }

    {
        wxAuiPaneInfo& pane=m_AuiMgr.GetPane(list_ctrl_info);
        if (pane.IsOk())
            pane.Caption(wxString(CSL_CAPTION_SERVER_INFO)+wxT(": ")+info->GetBestDescription());
    }
    {
        wxAuiPaneInfo& pane=m_AuiMgr.GetPane(player_info);
        if (pane.IsOk())
            pane.Caption(PlayerListGetCaption(info,true));
    }
    m_AuiMgr.Update();
}

void CslFrame::OnListItemActivated(wxListEvent& event)
{
    event.Skip();

    if (event.GetEventObject()==(void*)list_ctrl_info)
        return;

    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    if (info)
        ConnectToServer(info);
}

void CslFrame::OnTreeSelChanged(wxTreeEvent& event)
{
    event.Skip();

    wxTreeItemId item=event.GetItem();
    wxTreeItemId olditem=event.GetOldItem();

    if (olditem.IsOk())
    {
        wxString s=tree_ctrl_games->GetItemText(olditem);
        s=s.BeforeLast(wxT('('));
        tree_ctrl_games->SetItemText(olditem,s);
    }

    if (!item.IsOk())
        return;

    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);

    CslGame *game=data->GetGame();
    CslMaster *master=data->GetMaster();

    vector<CslServerInfo*>* servers;

    if (!game)
    {
        CslMenu::EnableItem(MENU_DEL);
        CslMenu::EnableItem(MENU_UPDATE);

        servers=&master->GetServers();

        //find the game
        item=tree_ctrl_games->GetItemParent(item);
        if (!item.IsOk())
            return;

        data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
        game=data->GetGame();
    }
    else
    {
        CslMenu::EnableItem(MENU_DEL,false);
        CslMenu::EnableItem(MENU_UPDATE,false);
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
    const wxTreeItemId& item=event.GetItem();
#ifdef __WXMSW__
    tree_ctrl_games->SelectItem(item);
#endif
    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
    CslMaster *master=data->GetMaster();

    if (master)
    {
        bool b=master->GetGame()->GetDefaultMasterConnection()==master->GetConnection();
        CslMenu::EnableItem(MENU_DEL,!b);
        CslMenu::EnableItem(MENU_UPDATE);
    }
    else
    {
        CslMenu::EnableItem(MENU_DEL,false);
        CslMenu::EnableItem(MENU_UPDATE,false);
    }

    PopupMenu(menuMaster);

    event.Skip();
}

void CslFrame::OnCommandEvent(wxCommandEvent& event)
{
    wxInt32 id=event.GetId();

    switch (id)
    {
        case wxID_EXIT:
            ::wxGetApp().Shutdown(CslApp::CSL_SHUTDOWN_NORMAL);
            Close();
            break;

        case wxID_PREFERENCES:
        {
            CslDlgSettings *dlg=new CslDlgSettings(this);
            if (dlg->ShowModal()!=wxID_OK)
                break;
            m_timerUpdate=g_cslSettings->updateInterval/CSL_TIMER_SHOT;
            m_engine->SetUpdateInterval(g_cslSettings->updateInterval);
            SaveSettings();
#ifndef __WXMAC__
            ToggleTrayIcon();
#endif
            break;
        }

        case MENU_ADD:
        {
            if (event.GetEventObject()==list_ctrl_player_search)
            {
                CslServerInfo *info=(CslServerInfo*)event.GetClientData();
                if (info && !info->IsFavourite())
                {
                    info->SetFavourite();
                    list_ctrl_favourites->ListUpdateServer(info);
                    list_ctrl_favourites->ListSort();
                }
            }
            else
            {
                wxInt32 gameId=TreeGetSelectedGame()->GetId();
                CslDlgAddMaster dlg(this);
                dlg.InitDlg(&gameId);
                if (dlg.ShowModal()==wxID_OK)
                {
                    CslGame *game=m_engine->GetGames()[gameId];
                    CslMaster *master=game->GetMasters().last();
                    game->AddMaster(master);
                    TreeAddMaster(wxTreeItemId(),master,true);
                }
            }
            break;
        }

        case MENU_DEL:
        {
            if (event.GetEventObject()==menuMaster)
            {
                list_ctrl_master->ListClear();
                TreeRemoveMaster();
            }
            else
            {
                CslServerInfo *info=(CslServerInfo*)event.GetClientData();
                if (!info)
                    break;

                if (info==m_oldSelectedInfo)
                    m_oldSelectedInfo=NULL;

                if (m_extendedDlg->GetInfo()==info)
                {
                    info->PingExt(false);
                    m_extendedDlg->DoShow(NULL);
                }

                list_ctrl_player_search->RemoveServer(info);

                loopvrev(m_playerInfos)
                {
                    if (m_playerInfos[i]->ServerInfo()==info)
                    {
                        if (i==0)
                        {
                            m_playerInfos[i]->ServerInfo(NULL);
                            m_playerInfos[i]->UpdateData();
                            wxAuiPaneInfo& pane=m_AuiMgr.GetPane(m_playerInfos[i]);
                            if (pane.IsOk())
                            {
                                pane.Caption(PlayerListGetCaption(NULL,true));
                                m_AuiMgr.Update();
                            }
                        }
                        else
                        {
                            if (m_AuiMgr.DetachPane(m_playerInfos[i]))
                                m_AuiMgr.Update();

                            delete m_playerInfos[i];
                            m_playerInfos.remove(i);
                        }
                        info->PingExt(false);
                    }
                }

                info->GetGame().DeleteServer(info);
            }
            break;
        }

        case MENU_UPDATE:
            UpdateMaster();
            break;

        case MENU_GAME_SERVER_COUNTRY:
        {
            m_countryServers.setsize(0);

            CslGame *game=TreeGetSelectedGame();
            CslMaster *master=TreeGetSelectedMaster();

            if (game)
            {
                const vector<CslServerInfo*>& servers=master ? master->GetServers() : game->GetServers();

                pane_country->Reset(CslPanelCountry::MODE_SERVER,m_countryServers.length());
                PanelCountrySetCaption(NULL);
                TogglePane(MENU_VIEW_COUNTRY,true);

                loopv(servers)
                {
                    if (CslEngine::PingOk(*servers[i],g_cslSettings->updateInterval))
                        pane_country->UpdateData(servers[i]);
                }

            }

            break;
        }

        case MENU_GAME_PLAYER_COUNTRY:
        {
            loopv(m_countryServers) m_countryServers[i]->PingExt(false);
            m_countryServers.setsize(0);

            CslGame *game=TreeGetSelectedGame();
            CslMaster *master=TreeGetSelectedMaster();

            if (game)
            {
                const vector<CslServerInfo*>& servers=master ? master->GetServers() : game->GetServers();

                loopv(servers)
                {
                    if (CslEngine::PingOk(*servers[i],g_cslSettings->updateInterval) &&
                        servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
                    {
                        m_countryServers.add(servers[i]);
                        servers[i]->Search=true;
                        servers[i]->PingExt(true);
                    }
                }

                pane_country->Reset(CslPanelCountry::MODE_PLAYER_MULTI,m_countryServers.length());
                PanelCountrySetCaption(NULL);
                TogglePane(MENU_VIEW_COUNTRY,true);
            }

            break;
        }

        case MENU_VIEW_GAMES:
        case MENU_VIEW_SERVER_INFO:
        case MENU_VIEW_USER_CHAT:
        case MENU_VIEW_PLAYER_LIST:
        case MENU_VIEW_PLAYER_SEARCH:
        case MENU_VIEW_COUNTRY:
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

        case MENU_VIEW_TRAFFIC:
        {
            m_trafficDlg=new CslDlgTraffic(this);
            m_trafficDlg->ShowModal();
            break;
        }

        case MENU_VIEW_AUTO_SORT:
            g_cslSettings->autoSortColumns=event.IsChecked();
            list_ctrl_master->ListSort();
            list_ctrl_master->ToggleSortArrow();
            list_ctrl_favourites->ListSort();
            list_ctrl_favourites->ToggleSortArrow();
            break;

        case MENU_VIEW_RELAYOUT:
            m_AuiMgr.LoadPerspective(m_defaultLayout);
            m_AuiMgr.Update();
            break;

        case wxID_ABOUT:
        {
            CslDlgAbout *dlg=new CslDlgAbout(this);
            dlg->ShowModal();
            break;
        }

        case CSL_BUTTON_SEARCH_CLOSE:
            g_cslSettings->showSearch=false;
            CslMenu::CheckItem(MENU_VIEW_SEARCH,false);
            ToggleSearchBar();
            break;

        case CSL_TEXT_SEARCH:
        {
            wxString s=text_ctrl_search->GetValue();

            if (s.IsEmpty())
            {
                button_search->Enable(false);
                gauge_search->Enable(false);
                SetSearchbarColour(false);

                list_ctrl_master->Highlight(-1,false,true);
                list_ctrl_favourites->Highlight(-1,false,true);
                list_ctrl_master->ListSearch(wxEmptyString);
                list_ctrl_favourites->ListSearch(wxEmptyString);
                if (radio_search_server->GetValue())
                    text_search_result->SetLabel(wxString::Format(_("Search result: %d servers"),0));
            }
            else
            {
                button_search->Enable();
                gauge_search->Enable();
            }

            if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED && radio_search_server->GetValue())
            {
                wxInt32 c=list_ctrl_master->ListSearch(s);
                c+=list_ctrl_favourites->ListSearch(s);

                SetSearchbarColour(!(c || s.IsEmpty()));

                text_search_result->SetLabel(wxString::Format(_("Search result: %d servers"),c));
                sizer_search->Layout();
            }
            else if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;
        }

        case CSL_BUTTON_SEARCH:
        {
            if (!radio_search_player->GetValue())
                break;

            loopv(m_searchedServers) m_searchedServers[i]->PingExt(false);
            m_searchedServers.setsize(0);

            m_searchString=text_ctrl_search->GetValue().Lower();
            m_searchResultPlayer=m_searchResultServer=0;

            gauge_search->SetValue(0);
            text_search_result->SetLabel(wxString::Format(_("Search result: %d players on %d servers"),0,0));

            list_ctrl_master->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER,false,true);
            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER,false,true);
            list_ctrl_player_search->ListClear();

            text_ctrl_search->SetFocus();

            if (m_searchString.IsEmpty())
                break;

            CslGame *game=TreeGetSelectedGame();
            if (!game)
                break;

            {
                const vector<CslServerInfo*>& servers=game->GetServers();
                loopv(servers)
                {
                    if (!servers[i]->IsFavourite() &&
                        CslEngine::PingOk(*servers[i],g_cslSettings->updateInterval) &&
                        servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
                    {
                        m_searchedServers.add(servers[i]);
                        servers[i]->Search=true;
                        servers[i]->PingExt(true);
                        list_ctrl_master->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,true,false,servers[i]);
                    }
                }
            }

            {
                vector<CslServerInfo*> servers;
                m_engine->GetFavourites(servers);
                loopv(servers)
                {
                    if (CslEngine::PingOk(*servers[i],g_cslSettings->updateInterval) &&
                        servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
                    {
                        m_searchedServers.add(servers[i]);
                        servers[i]->Search=true;
                        servers[i]->PingExt(true);
                        list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,true,false,servers[i]);
                    }
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
            list_ctrl_master->Highlight(-1,false,true);
            list_ctrl_favourites->Highlight(-1,false,true);

            wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED,CSL_TEXT_SEARCH);
            wxPostEvent(this,evt);
            break;
        }

        case CSL_RADIO_SEARCH_PLAYER:
        {
            button_search->Show();
            gauge_search->Show();
            text_ctrl_search->SetFocus();
            text_search_result->SetLabel(wxString::Format(_("Search result: %d players on %d servers"),0,0));
            sizer_search->Layout();
            list_ctrl_master->ListSearch(wxEmptyString);
            list_ctrl_favourites->ListSearch(wxEmptyString);
            list_ctrl_master->Highlight(-1,false,true);
            list_ctrl_favourites->Highlight(-1,false,true);
            SetSearchbarColour(false);

            wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED,CSL_TEXT_SEARCH);
            wxPostEvent(this,evt);
            break;
        }

        default:
            //events from list controls
            if (CSL_MENU_EVENT_IS_CONNECT(id))
            {
                CslServerInfo *info=(CslServerInfo*)event.GetClientData();

                if (info)
                {
                    wxInt32 pass=id==MENU_SERVER_CONNECT ?
                                 CslGameConnection::NO_PASS :
                                 CslGameConnection::ASK_PASS;
                    ConnectToServer(info,pass);
                }
            }
            else if (CSL_MENU_EVENT_IS_EXTINFO(id))
            {
                CslServerInfo *info=(CslServerInfo*)event.GetClientData();

                if (!info)
                    break;

                if (id==MENU_SERVER_EXT_FULL)
                {
                    info->PingExt(true);
                    m_extendedDlg->DoShow(info);
                }
                else
                    PlayerListCreateView(info,event.GetId()-MENU_SERVER_EXT_MICRO);
            }
            else if (CSL_MENU_EVENT_IS_URICOPY(id))
            {
                wxUint32 i;
                wxString s;
                CslServerInfo *info;
                bool con,add,pass=false;
                VoidPointerArray *servers=(VoidPointerArray*)event.GetClientData();

                for (i=0;i<servers->GetCount();i++)
                {
                    info=(CslServerInfo*)servers->Item(i);

                    if (!info->Password.IsEmpty())
                    {
                        pass=true;
                        break;
                    }
                }

                if (pass)
                {
                    pass=wxMessageBox(_("Copy server passwords too?"),_("Warning"),
                                      wxYES_NO|wxICON_WARNING,this)==wxYES;
                }

                for (i=0;i<servers->GetCount();i++)
                {
                    info=(CslServerInfo*)servers->Item(i);

                    if (!s.IsEmpty())
                        s<<wxT("\r\n");

                    con=id==MENU_SERVER_COPY_CON || id==MENU_SERVER_COPY_CONFAV;
                    add=id==MENU_SERVER_COPY_FAV || id==MENU_SERVER_COPY_CONFAV;

                    s<<CslIpcBase::CreateURI(*info,pass,con,add);
                }

                if (!s.IsEmpty() && wxTheClipboard->Open())
                {
                    wxTheClipboard->SetData(new wxTextDataObject(s));
                    wxTheClipboard->Close();
                }

                delete servers;
            }
            else if (CSL_MENU_EVENT_IS_NOTIFY(id))
            {
                wxUint32 flags=(wxUint32)-1;
                CslServerInfo *info=(CslServerInfo*)event.GetClientData();

                if (id==MENU_SERVER_NOTIFY_RESET)
                {
                    info->UnRegisterEvents(flags);
                    break;
                }

                flags=1<<(id-MENU_SERVER_NOTIFY_RESET-1);

                if (event.IsChecked())
                    info->RegisterEvents(flags);
                else
                    info->UnRegisterEvents(flags);
            }
            else if (CSL_MENU_EVENT_IS_LOCATION(id))
            {
                wxString *ip=(wxString*)event.GetClientData();
                CslGeoIPService *service=CslGeoIP::GetServices().Item(id-MENU_SERVER_LOCATION);
                ::wxLaunchDefaultBrowser(service->Host+service->Path+*ip,wxBROWSER_NEW_WINDOW);
                delete ip;
            }
            else if (CSL_MENU_EVENT_IS_FILTER(id))
                SetListCaption((wxInt32)(long)event.GetClientData());
    }
}

void CslFrame::OnKeypress(wxKeyEvent& event)
{
    event.Skip();

    static wxUint32 lastTicks=0;
    wxUint32 ticks=GetTicks();

    if (ticks-lastTicks<200)
        return;
    lastTicks=ticks;

    if (event.GetKeyCode()==WXK_ESCAPE)
    {
        if (CslGameConnection::IsWaiting())
        {
            CslServerInfo *info=CslGameConnection::GetInfo();

            list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);

            CslGameConnection::Reset();
        }
        else if (wxWindow::FindFocus()==text_ctrl_search)
        {
            text_ctrl_search->Clear();
            text_ctrl_search->SetFocus();

            wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED,CSL_TEXT_SEARCH);
            wxPostEvent(this,evt);
        }
    }
}

void CslFrame::OnSize(wxSizeEvent& event)
{
    static bool init=true;

    if (!IsMaximized() && event.GetSize()!=g_cslSettings->frameSizeMax)
    {
        if (init)
        {
            SetMinSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT));
            init=false;
        }
        g_cslSettings->frameSize=event.GetSize();
        m_maximised=false;
    }
    else
        m_maximised=true;

    event.Skip();
}

#ifndef __WXMAC__
void CslFrame::OnIconize(wxIconizeEvent& event)
{
#if wxCHECK_VERSION(2,9,0)
    bool iconized=event.IsIconized();
#else
    bool iconized=event.Iconized();
#endif
    if (g_cslSettings->systray&CSL_USE_SYSTRAY && iconized)
        Hide();

    event.Skip();
}

void CslFrame::OnTrayIcon(wxTaskBarIconEvent& WXUNUSED(event))
{
    ToggleShow();
}
#endif

void CslFrame::OnShow(wxShowEvent& event)
{
#if wxCHECK_VERSION(2,9,0)
    if (event.IsShown())
#else
    if (event.GetShow())
#endif
    {
        list_ctrl_master->ListAdjustSize(list_ctrl_master->GetClientSize());
        list_ctrl_favourites->ListAdjustSize(list_ctrl_favourites->GetClientSize());
    }
    else
        CslToolTip::ResetTip();

    event.Skip();
}

void CslFrame::OnClose(wxCloseEvent& event)
{
    if (event.GetEventObject()==m_extendedDlg)
    {
    }
    else if (event.GetEventObject()==m_outputDlg)
    {
        CslMenu::CheckItem(MENU_VIEW_OUTPUT,false);
        return;
    }
    else if (event.GetEventObject()==m_trafficDlg)
    {
        m_trafficDlg->Destroy();
        m_trafficDlg=NULL;
        return;
    }

    LOG_DEBUG("\n");

    if (::wxGetApp().Shutdown()!=CslApp::CSL_SHUTDOWN_FORCE)
    {
#ifndef __WXMAC__
        if (g_cslSettings->systray&CSL_USE_SYSTRAY &&
            g_cslSettings->systray&CSL_SYSTRAY_CLOSE &&
            ::wxGetApp().Shutdown()!=CslApp::CSL_SHUTDOWN_NORMAL)
        {
            Hide();
            return;
        }
        else
#endif
            if (CslGameConnection::IsPlaying())
            {
                wxMessageBox(_("There is currently a game running.\n"
                               _L_"Quit the game first and try again."),
                             _("Error"),wxOK|wxICON_ERROR,this);
                return;
            }
    }

    LOG_DEBUG("closing\n");

    event.Skip();
}

void CslFrame::OnPaneClose(wxAuiManagerEvent& event)
{
    CslServerInfo *info;

    if (event.pane->name.StartsWith(wxT("players")))
    {
        loopv(m_playerInfos)
        {
            if (m_playerInfos[i]==event.pane->window)
            {
                if ((info=m_playerInfos[i]->ServerInfo())!=NULL)
                    info->PingExt(false);
                if (i==0)
                {
                    CslMenu::CheckItem(MENU_VIEW_PLAYER_LIST,false);
                    m_playerInfos[i]->ServerInfo(NULL);
                }
                else
                    m_playerInfos.remove(i);
                break;
            }
        }
    }
    else if (event.pane->name==wxT("games"))
        CslMenu::CheckItem(MENU_VIEW_GAMES,false);
    else if (event.pane->name==wxT("info"))
        CslMenu::CheckItem(MENU_VIEW_SERVER_INFO,false);
    else if (event.pane->name==wxT("playerlist"))
        CslMenu::CheckItem(MENU_VIEW_PLAYER_LIST,false);
    else if (event.pane->name==wxT("search"))
        CslMenu::CheckItem(MENU_VIEW_PLAYER_SEARCH,false);
    else if (event.pane->name==wxT("country"))
        CslMenu::CheckItem(MENU_VIEW_COUNTRY,false);
    else if (event.pane->name==wxT("irc"))
        CslMenu::CheckItem(MENU_VIEW_USER_CHAT,false);
    else if (event.pane->name==wxT("favlist"))
        CslMenu::CheckItem(MENU_VIEW_FAVOURITES,false);
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

void CslFrame::OnToolTip(CslToolTipEvent& event)
{
    event.Title=m_toolTipTitle;
    event.Text.Add(m_toolTipTextLeft);
    event.Text.Add(m_toolTipTextRight);
    event.Pos=::wxGetClientDisplayRect().GetBottomRight();
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
                                                                    _L_"of Cube Server Lister available.\n"),
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

    CslGameConnection::Reset();

    list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
    list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
    list_ctrl_info->UpdateInfo(info);
}

void CslFrame::OnIPC(CslIpcEvent& event)
{
    wxString error;

    switch (event.Type)
    {
        case CslIpcEvent::IPC_DISCONNECT:
            if (m_ipcServer)
                m_ipcServer->Disconnect();
            break;

        case CslIpcEvent::IPC_COMMAND:
            if (!event.Request.IsEmpty())
            {
                LOG_DEBUG("IPC request: %s\n",U2A(event.Request));

                if (event.Request.Cmp(wxT("show"))==0)
                    ToggleShow();
                else
                {
                    wxInt32 i;
                    wxString s,token;
                    wxString host,pass;
                    CslGame *game=NULL;
                    wxUint16 port=0,iport=0;
                    bool connect=false,addfavourites=false;

                    wxURI uri(event.Request);

                    if (!uri.HasServer())
                    {
                        error=_("Invalid URI: No server specified.");
                        break;
                    }
                    if (!uri.HasQuery())
                    {
                        error=_("Invalid URI: No action specified.");
                        break;
                    }

                    wxStringTokenizer tkz(uri.GetQuery(),wxT("&"));

                    while (tkz.HasMoreTokens())
                    {
                        token=tkz.GetNextToken();

                        if ((i=token.Find(wxT("=")))==wxNOT_FOUND)
                            continue;

                        if (token.Mid(0,i).CmpNoCase(CSL_URI_INFOPORT_STR)==0)
                            iport=wxAtoi(token.Mid(i+1));
                        else if (token.Mid(0,i).CmpNoCase(CSL_URI_GAME_STR)==0)
                        {
                            s=token.Mid(i+1);
                            s.Replace(wxT("%20"),wxT(" "));
                            game=m_engine->FindGame(s);
                        }
                        else if (token.Mid(0,i).CmpNoCase(CSL_URI_ACTION_STR)==0)
                        {
                            s=token.Mid(i+1);

                            if (s.CmpNoCase(CSL_URI_ACTION_CONNECT_STR)==0)
                                connect=true;
                            else if (s.CmpNoCase(CSL_URI_ACTION_ADDFAV_STR)==0)
                                addfavourites=true;
                        }
                    }

                    if (!game)
                    {
                        error=_("Invalid URI: Unknown game.");
                        break;
                    }

                    if (addfavourites || connect)
                    {
                        wxIPV4address addr;
                        CslServerInfo *info;

                        if (CSL_CAP_CONNECT_PASS(game->GetCapabilities()))
                        {
                            pass=uri.GetUserInfo();
                            pass.Replace(wxT("%20"),wxT(" "));
                        }

                        host=uri.GetServer();
                        if (!(port=wxAtoi(uri.GetPort())))
                            port=game->GetDefaultGamePort();
                        if (!iport)
                            iport=game->GetInfoPort(port);

                        addr.Service(iport);
                        addr.Hostname(host);
                        addr.Hostname();

                        if (!(info=game->FindServerByAddr(addr)))
                        {
                            info=new CslServerInfo;
                            info->Create(game,host,port,iport,pass);
                            info->View=CslServerInfo::CSL_VIEW_DEFAULT;

                            if (!game->AddServer(info))
                            {
                                delete info;
                                break;
                            }
                        }
                        else if (!pass.IsEmpty())
                            info->Password=pass;

                        if (addfavourites)
                        {
                            if (!info->IsFavourite())
                            {
                                info->SetFavourite();
                                list_ctrl_favourites->ListUpdateServer(info);
                            }
                        }

                        if (connect)
                            ConnectToServer(info,pass.IsEmpty() ?
                                            CslGameConnection::NO_PASS :
                                            CslGameConnection::USE_PASS);
                    }
                    else
                        error=_("Invalid URI: Unknown action.");
                }
            }

            break;

        default:
            break;
    }

    if (!error.IsEmpty())
        wxMessageBox(error,_("Error!"),wxICON_ERROR,this);
}
