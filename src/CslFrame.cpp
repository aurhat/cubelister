/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
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
#include "CslEngine.h"
#include "CslApp.h"
#include "CslGeoIP.h"
#include "CslDlgAbout.h"
#include "CslDlgAddMaster.h"
#include "CslDlgConnectWait.h"
#include "CslDlgGeneric.h"
#include "CslFrame.h"

#define CSL_TIMER_SHOT 250


BEGIN_EVENT_TABLE(CslFrame,wxFrame)
    CSL_EVT_PONG(CslPongEvent::PLAYERSTATS, CslFrame::OnPong)
    CSL_EVT_PONG(CslPongEvent::TEAMSTATS, CslFrame::OnPong)
    CSL_EVT_PONG(CslPongEvent::EVENT, CslFrame::OnPong)
    EVT_TIMER(wxID_ANY,CslFrame::OnTimer)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslFrame::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslFrame::OnListItemActivated)
    EVT_TREE_SEL_CHANGED(wxID_ANY,CslFrame::OnTreeSelChanged)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,CslFrame::OnTreeRightClick)
    EVT_MENU(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_TEXT(SEARCH_TEXT, CslFrame::OnCommandEvent)
    EVT_TEXT_ENTER(SEARCH_TEXT, CslFrame::OnCommandEvent)
    EVT_BUTTON(SEARCH_BUTTON_SEARCH,CslFrame::OnCommandEvent)
    #ifndef __WXMAC__
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


bool g_csl_about_shown    = false;
bool g_csl_settings_shown = false;


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


#ifndef __WXMAC__
class CslTaskBarIcon : public wxTaskBarIcon
{
    public:
        CslTaskBarIcon(wxEvtHandler *parent,
                       const wxIcon& icon = wxNullIcon,
                       const wxString& tooltip = wxEmptyString):
                m_parent(parent)
        {
            if (!icon.IsNull() && icon.IsOk())
                SetIcon(icon, tooltip);
        }

        wxMenu* CreatePopupMenu()
        {
            wxMenu *menu = new wxMenu();
            bool iconized = ((wxTopLevelWindow*)wxTheApp->GetTopWindow())->IsIconized();

            CslMenu::AddItem(*menu, iconized ? wxID_RESTORE_FRAME : wxID_ICONIZE_FRAME,
                             iconized ? _("Show") : _("Hide"), wxART_NONE);
            menu->AppendSeparator();
            CslMenu::AddItem(*menu, wxID_ABOUT, _("About"), wxART_ABOUT);
            CslMenu::AddItem(*menu, wxID_PREFERENCES, _("Settings"), wxART_SETTINGS);
            menu->AppendSeparator();
            CslMenu::AddItem(*menu, wxID_EXIT, _("Exit"), wxART_QUIT);

            CslMenu::EnableItem(*menu, wxID_ABOUT, !g_csl_about_shown);
            CslMenu::EnableItem(*menu, wxID_PREFERENCES, !g_csl_settings_shown);

            return menu;
        }

        private:
            // don't push the events to the parents handler queue
            // but process them, so the taskbar icon is blocked
            void OnCommandEvent(wxCommandEvent& event)
            {
                   m_parent->ProcessEvent(event);
            }
            void OnLeftDown(wxTaskBarIconEvent& event)
            {
                   m_parent->ProcessEvent(event);
            }
            DECLARE_EVENT_TABLE()

            wxEvtHandler *m_parent;
};

BEGIN_EVENT_TABLE(CslTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(wxID_ANY, CslTaskBarIcon::OnCommandEvent)
    EVT_TASKBAR_LEFT_DOWN(CslTaskBarIcon::OnLeftDown)
END_EVENT_TABLE()
#endif // __WXMAC__


CslFrame::CslFrame(wxWindow* parent,int id,const wxString& title,
                   const wxPoint& pos,const wxSize& size,long style):
        wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE),
        CslPluginMgr(CSL_PLUGIN_TYPE_GUI)
{
    m_engine = ::wxGetApp().GetCslEngine();

    if (!m_engine->Init(this, CSL_UPDATE_INTERVAL_MIN, 1000/CSL_TIMER_SHOT))
    {
        wxMessageBox(    _("Failed to initialise internal engine!\n"
                     wxT_2("Please try to restart the application.")),
                     _("Fatal error!"), wxICON_ERROR, this);
        ::wxGetApp().Shutdown(CslApp::CSL_SHUTDOWN_ERROR);
        Close();
        return;
    }

#ifndef __WXMAC__
    m_taskbar_icon = NULL;
#endif
    m_oldSelectedInfo = NULL;

    CslSettings::LoadSettings();

    if (!LoadLocators())
    {
        CslGeoIP::AddService(wxT("GeoIPTool"),
                             wxT("http://geoiptool.com/"),
                             wxT("?IP="));
        CslGeoIP::AddService(wxT("MaxMind (Demo)"),
                             wxT("http://www.maxmind.com/"),
                             wxT("app/locate_demo_ip?ips="));
        CslGeoIP::AddService(wxT("NetIP"),
                             wxT("http://www.netip.de/"),
                             wxT("search?query="));
    }

    CslTTSSettings tts(CslGetSettings().TTS, CslGetSettings().TTSVolume);
    CslTTS::Init(tts);

    CreateControls();
    SetProperties();
    CslSettings::LoadServers();
    DoLayout();

    m_engine->SetUpdateInterval(CslGetSettings().UpdateInterval);

    CslGame *game;
    const CslArrayCslGame& games=m_engine->GetGames();

    loopv(games)
    {
        game=games[i];

        CslMaster *master = CslMaster::Create(game->GetDefaultMasterConnection());

        if (game->AddMaster(master)<0)
            CslMaster::Destroy(master);

        bool select = CslGetSettings().LastGame.IsEmpty() ? i==0 :
                      CslGetSettings().LastGame==game->GetName();

        const CslGameIcon *icon = game->GetIcon(24);

        wxBitmap bmp = icon ?
            BitmapFromData(Csl2wxBitmapType(icon->Type), icon->Data, icon->DataSize) :
            wxNullBitmap;

        m_imgListTree.Add(bmp.IsOk() ? bmp : wxBitmap(24, 24));

        TreeGamesAddGame(game, bmp.IsOk() ? i+1 : -1, select);

        TreeGamesScrollToSelection();
    }

    CslMenu::EnableItem(MENU_ADD);

    m_timerUpdate=CslGetSettings().UpdateInterval/CSL_TIMER_SHOT;
    m_timerCount=0;
    m_timerInit=true;
    m_timer.SetOwner(this);
    m_timer.Start(CSL_TIMER_SHOT);

    m_ipcServer = new CslIpcServer(this);
    if (!m_ipcServer->Create(CSL_IPC_SERV))
    {
        CSL_LOG_DEBUG("couldn't create IPC server socket\n");
        delete m_ipcServer;
        m_ipcServer = NULL;
    }

    m_versionCheckThread = new CslVersionCheckThread(this);

    if (m_versionCheckThread->IsOk())
        m_versionCheckThread->Run();

    LoadPlugins(this);
}

CslFrame::~CslFrame()
{
    if (!m_engine->IsOk())
        return;

#ifndef __WXMAC__
    // delete the icon immediately so it doesn't receive any
    // mouse events anymore which causes crash at least on __WXMSW__
    if (m_taskbar_icon)
        delete m_taskbar_icon;
#endif

    if (m_timer.IsRunning())
        m_timer.Stop();

    CslSettings::SaveSettings();
    CslSettings::SaveServers();
    SaveLocators();

    CslToolTip::Reset();

    if (CslGameConnection::IsPlaying())
    {
        wxString error;
        CslGameConnection::GetInfo()->GetGame().GameEnd(error);
    }

    UnloadPlugins(this);

    if (m_ipcServer)
    {
        delete m_ipcServer;
        m_ipcServer=NULL;
    }

    CslTTS::DeInit();

    if (m_engine->IsOk())
        m_engine->DeInit();

    if (m_versionCheckThread)
        delete m_versionCheckThread;

    m_aui.UnInit();
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
    CslMenu::AddItem(*menu, wxID_PREFERENCES, _("&Settings"), wxART_SETTINGS);
    CslMenu::AddItem(*menu, wxID_EXIT, _("&Exit"), wxART_QUIT);
#endif

    menuMaster=new wxMenu();
    CslMenu::AddItem(*menuMaster, MENU_UPDATE,              _("&Update from master\tF5"), wxART_RELOAD);
    menuMaster->AppendSeparator();
    CslMenu::AddItem(*menuMaster, MENU_ADD,                 _("Add a &new master ..."),   wxART_ADD_BOOKMARK);
    CslMenu::AddItem(*menuMaster, MENU_DEL,                 _("&Remove master"),          wxART_DEL_BOOKMARK);
    CslMenu::AddItem(*menuMaster, MENU_GAME_SERVER_COUNTRY, _("&Servers by country"),     wxART_COUNTRY_UNKNOWN);
    CslMenu::AddItem(*menuMaster, MENU_GAME_PLAYER_COUNTRY, _("&Players by country"),     wxART_COUNTRY_UNKNOWN);
    menuMaster->AppendSeparator();
    CslMenu::AddItem(*menuMaster, MENU_GAME_SEARCH_LAN,     _("&Search for LAN servers"), wxART_NONE, wxITEM_CHECK);
    menubar->Append(menuMaster, _("&Game"));
#ifdef __WXMAC__
    CslMenu::AddItem(*menuMaster, wxID_PREFERENCES,         _("&Settings"),               wxART_SETTINGS);
    CslMenu::AddItem(*menuMaster, wxID_EXIT,                _("&Exit"),                   wxART_QUIT);
    CslMenu::AddItem(*menuMaster, wxID_ABOUT,               _("A&bout"),                  wxART_ABOUT);
#endif

    menu=new wxMenu();
    CslMenu::AddItem(*menu, MENU_VIEW_GAMES,         _("Show games"),                wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_SERVER_INFO,   _("Show server information"),   wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_PLAYER_LIST,   _("Show players"),              wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_SEARCH,        _("Show search\tCTRL+F"),       wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_TRAFFIC,       _("Show Traffic"),              wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_PLAYER_SEARCH, _("Show player search result"), wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_COUNTRY,       _("Show countries list"),       wxART_NONE, wxITEM_CHECK);
    CslMenu::AddItem(*menu, MENU_VIEW_FAVOURITES,    _("Show favourites"),           wxART_NONE, wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(*menu, MENU_VIEW_OUTPUT,        _("Show game output\tCTRL+O"),  wxART_NONE, wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(*menu, MENU_VIEW_AUTO_SORT,     _("Auto-sort\tCTRL+L"),         wxART_NONE, wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItem(*menu, MENU_VIEW_RELAYOUT,      _("Reset layout"),              wxART_NONE);
    menubar->Append(menu,_("&View"));

#ifndef __WXMAC__
    menu=new wxMenu();
    CslMenu::AddItem(*menu, wxID_ABOUT, _("A&bout"), wxART_ABOUT);
    menubar->Append(menu, _("&Help"));
#endif

    SetMenuBar(menubar);

    CslMenu::SetMainMenu(menubar);

    CslMenu::EnableItem(MENU_ADD,false);
    CslMenu::EnableItem(MENU_DEL,false);
    CslMenu::EnableItem(MENU_UPDATE,false);
    CslMenu::CheckItem(MENU_GAME_SEARCH_LAN, CslGetSettings().SearchLAN);
    CslMenu::CheckItem(MENU_VIEW_AUTO_SORT, CslGetSettings().AutoSortColumns);
}

wxInt32 CslFrame::GetFreeId()
{
    static wxInt32 id=wxID_ANY;

    if (id==wxID_ANY)
        wxRegisterId((id = CSL_PLUGIN_EVENT_ID_MIN_GUI));

    return wxNewId();
}

wxInt32 CslFrame::GetFreeIds(wxInt32 count, wxInt32 ids[])
{
    if (count<=0)
        return -1;

    loopi(count)
        ids[i] = GetFreeId();

    return ids[0];
}

wxMenu* CslFrame::GetPluginMenu()
{
    wxInt32  menuid;
    wxMenuBar *menubar = GetMenuBar();

    if ((menuid = menubar->FindMenu(_("&Plugins")))!=wxNOT_FOUND)
        return menubar->GetMenu(menuid);
    else
    {
        wxMenu *menu = new wxMenu;
        menubar->Insert(3, menu, _("&Plugins"));
        return menu;
    }

    return NULL;
}

void CslFrame::CreateControls()
{
    long list_style = wxLC_REPORT;
    long tree_style = wxTR_NO_LINES|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT|wxTR_DEFAULT_STYLE;

#ifdef __WXMSW__
    CSL_FLAG_SET(list_style, wxNO_BORDER);
    CSL_FLAG_SET(tree_style, wxNO_BORDER);
#else
    CSL_FLAG_SET(list_style, wxSUNKEN_BORDER);
    CSL_FLAG_SET(tree_style, wxSUNKEN_BORDER);
#endif // __WXMSW__

    wxArtProvider::Push(new CslArt);

    CslListCtrl::CreateImageList(m_engine);

    m_imgListTree.Create(24,24,true);
    m_imgListTree.Add(GET_ART_TOOLBAR(wxART_MASTER));

    tree_ctrl_games = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, tree_style);
    list_ctrl_info = new CslListCtrlInfo(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, list_style|wxLC_NO_HEADER|wxLC_HRULES);

    player_info = new CslPanelServerView(this, wxT("players0"), list_style);
    m_serverViews.push_back(player_info);

    pane_search = new CslPanelSearch(this);

    pane_traffic = new CslPanelTraffic(this);

    pane_country=new CslPanelCountry(this, list_style);
    list_ctrl_player_search = new CslListCtrlPlayerSearch(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, list_style);

    list_ctrl_master = new CslListCtrlServer(this, CslListCtrlServer::CSL_LIST_MASTER, wxDefaultPosition, wxDefaultSize,
                                             list_style, wxDefaultValidator, wxT("master"));
    list_ctrl_favourites = new CslListCtrlServer(this, CslListCtrlServer::CSL_LIST_FAVOURITE, wxDefaultPosition, wxDefaultSize,
                                                 list_style, wxDefaultValidator, wxT("favourites"));

    m_outputDlg = new CslDlgOutput(this);
    m_extendedDlg = new CslDlgExtended(this);

    SetStatusBar(CslStatusBar::Init(this));

    CreateMainMenu();
}

void CslFrame::SetProperties()
{
#ifndef __WXMSW__
    m_aui.SetFlags(wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_HINT_FADE |
                   wxAUI_MGR_VENETIAN_BLINDS_HINT | wxAUI_MGR_NO_VENETIAN_BLINDS_FADE);
#endif
    m_aui.SetManagedWindow(this);

    SetTitle(_("Cube Server Lister"));

    player_info->ListCtrl()->ListInit(CslListCtrlPlayerView::SIZE_MINI);

    list_ctrl_master->Init(list_ctrl_favourites, &CslGetSettings().FilterMaster);
    list_ctrl_favourites->Init(list_ctrl_master, &CslGetSettings().FilterFavourites);

    Connect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(CslFrame::OnAuiPaneClose), NULL, this);

    // depending on platform, key events from childs never reach
    // the main frame using static event tables - connect them here
    ConnectEventRecursive(wxID_ANY, this, this, wxEVT_KEY_DOWN, wxKeyEventHandler(CslFrame::OnKeypress));

    tree_ctrl_games->SetImageList(&m_imgListTree);
    m_treeGamesRoot = tree_ctrl_games->AddRoot(wxEmptyString, -1, -1);

#ifdef __WXMSW__
    SetIcon(wxICON(appicon));
#else
    wxIcon icon;
    icon.CopyFromBitmap(GET_ART_FRAME(wxART_CSL));
    SetIcon(icon);
#endif
}

void CslFrame::DoLayout()
{
    wxSize size=list_ctrl_master->GetBestSize();

    m_aui.AddPane(list_ctrl_master, wxAuiPaneInfo().Name(wxT("masterlist")).
                  CloseButton(false).Center().BestSize(size).MinSize(size.x, 20));
    m_aui.AddPane(list_ctrl_favourites, wxAuiPaneInfo().Name(wxT("favlist")).
                  Bottom().Row(2).BestSize(size).MinSize(size).FloatingSize(600, 240));
    m_aui.AddPane(tree_ctrl_games, wxAuiPaneInfo().Name(wxT("games")).
                  Left().Layer(2).Position(0).
                  MinSize(300, 20).BestSize(300, 240).FloatingSize(300, 240));
    m_aui.AddPane(list_ctrl_info, wxAuiPaneInfo().Name(wxT("info")).
                  Left().Layer(2).Position(1).Fixed());
    m_aui.AddPane(player_info, wxAuiPaneInfo().Name(wxT("players0")).
                  Left().Layer(2).Position(2).
                  MinSize(300, 20).BestSize(CslListCtrlPlayerView::BestSizeMini).FloatingSize(300, 240));
    m_aui.AddPane(pane_search, wxAuiPaneInfo().Name(wxT("searchmain")).
                  Left().Layer(2).Position(3).Hide().Fixed());
    m_aui.AddPane(pane_traffic, wxAuiPaneInfo().Name(wxT("traffic")).
                  Left().Layer(2).Position(4).Hide().Fixed());
    m_aui.AddPane(list_ctrl_player_search, wxAuiPaneInfo().Name(wxT("search")).
                  Left().Layer(1).Position(0).Hide().
                  MinSize(300, 80).BestSize(300, 240).FloatingSize(300, 240));
    m_aui.AddPane(pane_country, wxAuiPaneInfo().Name(wxT("country")).
                  Left().Layer(1).Position(1).Hide().
                  MinSize(300, 80).BestSize(300, 240).FloatingSize(300, 240));

    m_aui_default_layout = m_aui.SavePerspective();

    if (!CslGetSettings().Layout.IsEmpty())
        m_aui.LoadPerspective(CslGetSettings().Layout, false);

    SetAuiPaneSettings();

    ServerBrowserSetCaption(CslListCtrlServer::CSL_LIST_MASTER);
    ServerBrowserSetCaption(CslListCtrlServer::CSL_LIST_FAVOURITE);

    m_aui.Update();

    if (CslGetSettings().FrameSizeMax!=wxDefaultSize)
    {
        m_maximised=true;
        SetMinSize(CslGetSettings().FrameSize);
#ifdef __WXGTK__
        SetSize(CslGetSettings().FrameSizeMax);
#endif
        Maximize(true);
    }
    else
    {
        m_maximised=false;
        SetMinSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT));
        SetSize(CslGetSettings().FrameSize);
    }

#ifdef __WXMSW__
    CentreOnScreen();
#endif

    //connect after calling size functions, so CslGetSettings().frameSize has the right value
    Connect(wxEVT_SIZE,wxSizeEventHandler(CslFrame::OnSize),NULL,this);

#ifndef __WXMAC__
    ToggleTrayIcon();
#endif
}

void CslFrame::SetAuiPaneSettings()
{
    #define CSL_CAPTION_GAMES                  _("Games")
    #define CSL_CAPTION_MASTER_LIST_SERVERS    _("Master list servers")
    #define CSL_CAPTION_FAVLAN_LIST_SERVERS    _("Favourites & LAN servers")
    #define CSL_CAPTION_PLAYERS_SELECTED       _("Selected server")
    #define CSL_CAPTION_TRAFFIC                _("Traffic")
    #define CSL_CAPTION_SEARCH                 _("Search Servers")
    #define CSL_CAPTION_PLAYER_SEARCH          _("Player search result")
    #define CSL_CAPTION_SERVER_COUNTRY         _("Server countries")
    #define CSL_CAPTION_PLAYER_COUNTRY         _("Player countries")
    #define CSL_CAPTION_SERVER_INFO            _("Server info")

    wxAuiPaneInfo *pane;

    pane=&m_aui.GetPane(tree_ctrl_games);

    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_GAMES);
        CslMenu::CheckItem(MENU_VIEW_GAMES, pane->IsShown());
    }

    pane=&m_aui.GetPane(list_ctrl_info);

    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_SERVER_INFO);
        CslMenu::CheckItem(MENU_VIEW_SERVER_INFO, pane->IsShown());
        AuiResizeFixedPane(*pane, list_ctrl_info);
    }

    pane=&m_aui.GetPane(player_info);

    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYERS_SELECTED);
        CslMenu::CheckItem(MENU_VIEW_PLAYER_LIST,pane->IsShown());
    }

    pane=&m_aui.GetPane(pane_search);

    if (pane->IsOk())
    {
        // force new pane settings
        if (CslGetSettings().Version<4)
            pane->Float();

        pane->Caption(CSL_CAPTION_SEARCH);
        CslMenu::CheckItem(MENU_VIEW_SEARCH, pane->IsShown());
        AuiResizeFixedPane(*pane, pane_search);
    }

    pane=&m_aui.GetPane(pane_traffic);

    if (pane->IsOk())
    {
        // force new pane settings
        if (CslGetSettings().Version<3)
            pane->Float();

        pane->Caption(CSL_CAPTION_TRAFFIC);
        CslMenu::CheckItem(MENU_VIEW_TRAFFIC, pane->IsShown());
        AuiResizeFixedPane(*pane, pane_traffic);
    }

    pane=&m_aui.GetPane(list_ctrl_master);

    if (pane->IsOk())
        pane->Caption(CSL_CAPTION_MASTER_LIST_SERVERS);

    pane=&m_aui.GetPane(list_ctrl_favourites);

    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_FAVLAN_LIST_SERVERS);
        CslMenu::CheckItem(MENU_VIEW_FAVOURITES,pane->IsShown());
    }

    pane=&m_aui.GetPane(list_ctrl_player_search);

    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYER_SEARCH);
        CslMenu::CheckItem(MENU_VIEW_PLAYER_SEARCH,pane->IsShown());
    }

    pane=&m_aui.GetPane(pane_country);

    if (pane->IsOk())
    {
        pane->Caption(CSL_CAPTION_PLAYER_COUNTRY);
        CslMenu::CheckItem(MENU_VIEW_COUNTRY,pane->IsShown());
    }
}

wxAuiPaneInfo& CslFrame::AuiResizeFixedPane(wxAuiPaneInfo& pane, wxWindow *window)
{
    // ignore the loaded layout size for a fixed size pane
    // wxAUI calculates the size while adding the pane, so
    // copy the pane settings, remove it, reset the size
    // and add it again

    wxAuiPaneInfo copy;
    copy.SafeSet(pane);

    m_aui.DetachPane(window);
    copy.MinSize(wxDefaultSize).BestSize(wxDefaultSize).FloatingSize(wxDefaultSize).Fixed();
    m_aui.AddPane(window, copy);

    return m_aui.GetPane(window);
}

void CslFrame::CountryViewSetCaption(CslServerInfo *info)
{
    wxString caption;
    wxUint32 mode = pane_country->GetMode();

    caption = mode==CslPanelCountry::MODE_SERVER ?
                  CSL_CAPTION_SERVER_COUNTRY :
                  CSL_CAPTION_PLAYER_COUNTRY;

    caption << wxT(": ");

    if (!info || mode==CslPanelCountry::MODE_SERVER)
    {
        CslGame *game = TreeGamesGetSelectedGame();
        CslMaster *master = TreeGamesGetSelectedMaster();

        if (!game)
            return;

        if (master)
            caption << master->GetConnection().Address << wxT(" (") << game->GetName() << wxT(")");
        else
            caption << game->GetName();
    }
    else
        caption << info->GetBestDescription() << wxT(" (")+info->GetGame().GetName() << wxT(")");

    wxAuiPaneInfo& pane = m_aui.GetPane(pane_country);

    if (pane.IsOk())
    {
        pane.Caption(caption);

        m_aui.Update();
    }
}

void CslFrame::ServerBrowserSetCaption(wxInt32 id, const wxString& addon)
{
    wxWindow *panel;
    wxUint32 filter;
    wxString caption;

    if (id==CslListCtrlServer::CSL_LIST_MASTER)
    {
        panel = list_ctrl_master;
        filter = CslGetSettings().FilterMaster;
        caption = CSL_CAPTION_MASTER_LIST_SERVERS;
    }
    else if (id==CslListCtrlServer::CSL_LIST_FAVOURITE)
    {
        panel = list_ctrl_favourites;
        filter = CslGetSettings().FilterFavourites;
        caption = CSL_CAPTION_FAVLAN_LIST_SERVERS;
    }
    else
        return;

    wxAuiPaneInfo& pane = m_aui.GetPane(panel);

    if (!pane.IsOk())
        return;

    if (filter)
    {
        caption << wxT("  (") << _("Filters:") << wxT(" ");
        if (filter&CSL_FILTER_OFFLINE)
            caption << MENU_SERVER_FILTER_OFF_STR << (wxT(", "));
        if (filter&CSL_FILTER_FULL)
            caption << MENU_SERVER_FILTER_FULL_STR << (wxT(", "));
        if (filter&CSL_FILTER_EMPTY)
            caption << MENU_SERVER_FILTER_EMPTY_STR << (wxT(", "));
        if (filter&CSL_FILTER_NONEMPTY)
            caption << MENU_SERVER_FILTER_NONEMPTY_STR << (wxT(", "));
        if (filter&CSL_FILTER_MM2)
            caption << MENU_SERVER_FILTER_MM2_STR << (wxT(", "));
        if (filter&CSL_FILTER_MM3)
            caption << MENU_SERVER_FILTER_MM3_STR << (wxT(", "));

        caption.Remove(caption.Length()-2);
        caption << wxT(")");
    }

    caption << wxT(" ") << addon;

    pane.Caption(caption);

    m_aui.Update();
}

wxString CslFrame::ServerViewGetCaption(CslServerInfo *info, bool selected)
{
    wxString caption;

    if (selected)
    {
        caption << CSL_CAPTION_PLAYERS_SELECTED;

        if (info)
            caption << wxT(": ");
    }

    if (info)
        caption << info->GetBestDescription() << wxT(" (")+info->GetGame().GetName() << wxT(")");

    return caption;
}

void CslFrame::CreateServerView(CslServerInfo *info,wxUint32 view,const wxString& name)
{
    wxSize size;

    if (view==CslListCtrlPlayerView::SIZE_MICRO)
        size = CslListCtrlPlayerView::BestSizeMicro;
    else
        size = CslListCtrlPlayerView::BestSizeMini;

    wxAuiPaneInfo pane;
    wxString panename=name;

    pane.BestSize(size).DestroyOnClose().Caption(ServerViewGetCaption(info,false));

    if (panename.IsEmpty())
    {
        long l;
        wxInt32 j, id = 1;
        wxArrayInt ids;

        loopv(m_serverViews)
        {
            if (i==0)
                continue;

            wxAuiPaneInfo& p = m_aui.GetPane(m_serverViews[i]);

            if (p.IsOk() && p.name.Mid(7).ToLong(&l))
            {
                for (j=0; j<(wxInt32)ids.size(); j++)
                {
                    if (l<ids[j])
                        break;
                }
                ids.Insert(l, j);
            }
        }

        loopv(ids)
        {
            if (id<ids[i])
                break;
            else
                id++;
        }

        pane.Float().FloatingPosition(::wxGetMousePosition());
        panename=wxString::Format(wxT("players%d"), id);
    }

    pane.Name(panename);

    long style = wxLC_REPORT;

#ifdef __WXMSW__
    CSL_FLAG_SET(style, wxNO_BORDER);
#else
    CSL_FLAG_SET(style, wxSUNKEN_BORDER);
#endif // __WXMSW__

    // lock the main pane until aui manages the new pane
    wxWindowUpdateLocker lock(this);

    CslPanelServerView *playerinfo = new CslPanelServerView(this, panename, style);
    playerinfo->ListCtrl()->ListInit(view);
    playerinfo->SetServerInfo(info);
    m_serverViews.push_back(playerinfo);

    m_aui.AddPane(playerinfo,pane);
    m_aui.Update();

    info->PingExt(true);
    m_engine->PingEx(info,true);
}

void CslFrame::ToggleAuiPane(wxInt32 id, bool force)
{
    wxAuiPaneInfo *pane;

    switch (id)
    {
        case MENU_VIEW_GAMES:
            pane=&m_aui.GetPane(tree_ctrl_games);
            break;
        case MENU_VIEW_SERVER_INFO:
            pane=&m_aui.GetPane(list_ctrl_info);
            break;
        case MENU_VIEW_SEARCH:
            pane=&m_aui.GetPane(pane_search);
            break;
        case MENU_VIEW_TRAFFIC:
            pane=&m_aui.GetPane(pane_traffic);
            break;
        case MENU_VIEW_PLAYER_LIST:
            pane=&m_aui.GetPane(player_info);
            break;
        case MENU_VIEW_PLAYER_SEARCH:
            pane=&m_aui.GetPane(list_ctrl_player_search);
            break;
        case MENU_VIEW_COUNTRY:
            pane=&m_aui.GetPane(pane_country);
            break;
        case MENU_VIEW_FAVOURITES:
            pane=&m_aui.GetPane(list_ctrl_favourites);
            break;
        default:
            return;
    }

    if (!pane->IsOk())
        return;

    bool shown = pane->IsShown();

    if (shown && force)
        return;

    if (!shown)
        pane->window->Show();

    pane->Show(!shown);
    CslMenu::CheckItem(id, !shown);

    m_aui.Update();
}

void CslFrame::ToggleShow(wxInt32 mode)
{
    bool iconized = IsIconized();

    if ((mode>0 && !iconized) ||
        (mode<0 && iconized))
        return;

    if (iconized)
    {
#ifndef __WXMAC__
        if (CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY))
            Show(true);
#endif
        Iconize(false);
    }
    else
    {
        Iconize(true);

#ifndef __WXMAC__
        if (CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY))
            Show(false);
#endif
    }
}

#ifndef __WXMAC__
void CslFrame::ToggleTrayIcon()
{
    if (CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY) && !m_taskbar_icon)
    {
        static wxIcon icon;
        static wxBitmap bmp = GET_ART_TOOLBAR(wxART_CSL);

        if (!icon.IsOk())
            icon.CopyFromBitmap(bmp);

        m_taskbar_icon = new CslTaskBarIcon(this, icon, CSL_NAME_STR);
    }
    else if (m_taskbar_icon && !CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY))
    {
        ToggleShow(1);

        delete m_taskbar_icon;
        m_taskbar_icon = NULL;
    }
}
#endif

void CslFrame::SetTotalPlaytime(CslGame* game)
{
    if (!game)
        return;

    wxString s;
    wxUint32 playtime=0;
    CslServerInfo *info=NULL;
    CslArrayCslServerInfo& servers=game->GetServers();

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
            s<<wxT("  -  ");
            s<<wxString::Format(_("most played server: \'%s\' (total play time: %s)"),
                                info->GetBestDescription().c_str(),
                                FormatSeconds(info->PlayTimeTotal).c_str());
        }
    }

    CslStatusBar::SetText(1,s);
}

bool CslFrame::TreeGamesScrollToSelection()
{
    wxTreeItemId item = tree_ctrl_games->GetSelection();

    if (item.IsOk())
    {
        tree_ctrl_games->ScrollTo(item);
        return true;
    }

    return false;
}

wxTreeItemId CslFrame::TreeGamesFindItem(wxTreeItemId parent, CslGame *game, CslMaster *master)
{
    wxTreeItemId item;

    if (tree_ctrl_games->GetCount()<1)
        return item;

    wxTreeItemIdValue cookie;

    item = tree_ctrl_games->GetFirstChild(parent, cookie);

    while (item.IsOk())
    {
        CslTreeItemData *data = (CslTreeItemData*)tree_ctrl_games->GetItemData(item);

        if (game)
        {
            if (data->GetGame()==game)
                break;
            else if (master && data->GetMaster()==master)
                break;
        }

        item = tree_ctrl_games->GetNextSibling(item);
    }

    return item;
}

void CslFrame::TreeGamesAddMaster(wxTreeItemId parent,CslMaster *master,bool focus)
{
    if (!parent.IsOk())
    {
        parent=TreeGamesFindItem(m_treeGamesRoot,master->GetGame(),NULL);
        if (!parent.IsOk())
        {
            TreeGamesAddGame(master->GetGame(),true);
            return;
        }
    }

    if (TreeGamesFindItem(parent,NULL,master).IsOk())
        return;

    wxTreeItemId item;
    CslTreeItemData *data=new CslTreeItemData(master);
    item=tree_ctrl_games->AppendItem(parent,master->GetConnection().Address,0,-1,data);

    if (focus)
        tree_ctrl_games->SelectItem(item);
}

void CslFrame::TreeGamesRemoveMaster()
{
    wxTreeItemId item;
    CslMaster *master=TreeGamesGetSelectedMaster(&item);

    if (!master)
        return;

    if (m_timer.IsRunning())
        m_timer.Stop();

    master->GetGame()->DeleteMaster(master->GetId());
    tree_ctrl_games->Delete(item);

    m_timer.Start(CSL_TIMER_SHOT);
}

void CslFrame::TreeGamesAddGame(CslGame *game, wxInt32 img, bool focus)
{
    if (!game)
    {
        wxASSERT(game!=NULL);
        return;
    }

    wxTreeItemId item = TreeGamesFindItem(m_treeGamesRoot, game, NULL);

    if (!item.IsOk())
    {
        CslTreeItemData *data = new CslTreeItemData(game);
        item = tree_ctrl_games->AppendItem(m_treeGamesRoot, game->GetName(), img, -1, data);

        if (focus)
            tree_ctrl_games->SelectItem(item);
    }

    CslArrayCslMaster& masters = game->GetMasters();
    loopv(masters) TreeGamesAddMaster(item,masters[i]);

    //tree_ctrl_games->Expand(item);
}

CslGame* CslFrame::TreeGamesGetSelectedGame(wxTreeItemId *item)
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

CslMaster* CslFrame::TreeGamesGetSelectedMaster(wxTreeItemId *item)
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
    CslMaster *master=TreeGamesGetSelectedMaster();
    if (!master)
        return;

    m_timer.Stop();

    CslGame *game = master->GetGame();

    if (game)
    {
        CslServerInfo *info;
        wxUint32 now=wxDateTime::Now().GetTicks();
        CslArrayCslServerInfo& servers=game->GetServers();

        loopvrev(servers)
        {
            info=servers[i];

            if (CslGetSettings().CleanupServers &&
                now-info->LastSeen>CslGetSettings().CleanupServers &&
                !(CslGetSettings().CleanupServersKeepFav && info->IsFavourite()) &&
                !(CslGetSettings().CleanupServersKeepStats && info->HasStats()))
            {
                loopvj(m_serverViews)
                {
                    if (j!=0 && m_serverViews[j]->GetServerInfo()==info)
                    {
                        info=NULL;
                        break;
                    }
                }

                if (info)
                {
                    CSL_LOG_DEBUG("Server cleanup: %s - %s (%s)\n", U2C(game->GetName()),
                                  U2C(info->GetBestDescription()), U2C(info->Address().GetIPString()));

                    if (m_serverViews.size() && m_serverViews[0]->GetServerInfo()==info)
                    {
                        m_serverViews[0]->SetServerInfo(NULL);
                        m_serverViews[0]->UpdateData();
                        wxAuiPaneInfo& pane=m_aui.GetPane(m_serverViews[0]);
                        if (pane.IsOk())
                        {
                            pane.Caption(ServerViewGetCaption(NULL,true));
                            m_aui.Update();
                        }
                    }

                    if (info->IsFavourite())
                        list_ctrl_favourites->RemoveServer(NULL,info,-1);

                    m_engine->DeleteServer(game, info);
                }
            }
        }
    }

    SetStatusText(_("Sending request to master: ")+master->GetConnection().Address,1);
    wxInt32 num=m_engine->UpdateFromMaster(master);
    if (num<0)
        SetStatusText(_("Error on update from master: ")+master->GetConnection().Address,1);
    else
    {
        m_timerCount=0;
        list_ctrl_master->Clear();
        SetStatusText(wxString::Format(_("Got %d servers from master"),num),1);
    }

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

    list_ctrl_info->UpdateServer(info);
    list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,true,false,info);
    list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,true,false,info);
}

wxUint32 CslFrame::LoadLocators()
{
    if (!::wxFileExists(CSL_LOCATORS_FILE))
        return 0;

    long int val;
    wxUint32 i=0;
    wxString Name,Host,Path;

    wxFileConfig config(wxT(""),wxT(""),CSL_LOCATORS_FILE,wxT(""),
                        wxCONFIG_USE_LOCAL_FILE,wxConvLocal);

    config.SetPath(wxT("/Version"));
    config.Read(wxT("Version"), &val, 0);

    for (;;)
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

void CslFrame::OnPong(CslPongEvent& event)
{
    event.Skip();

    CslServerInfo *info=event.GetServerInfo();

    if (!info)
        return;

    wxInt32 type=event.GetId();

    if (type==CslPongEvent::EVENT)
    {
        if (info->HasEvents())
        {
#define TT_SPACER (s.IsEmpty() ? wxT(" "):wxT(", "))
            wxString s;

            if (info->HasEvent(CslServerEvents::EVENT_EMPTY))
                s<<wxT(" ")<<wxT("empty");
            if (info->HasEvent(CslServerEvents::EVENT_NOT_EMPTY))
                s<<TT_SPACER<<wxT("not empty anymore");

            if (info->HasEvent(CslServerEvents::EVENT_FULL))
                s<<TT_SPACER<<wxT("full");
            if (info->HasEvent(CslServerEvents::EVENT_NOT_FULL))
                s<<TT_SPACER<<wxT("not full anymore");

            if (info->HasEvent(CslServerEvents::EVENT_LOCKED))
                s<<TT_SPACER<<wxT("in locked mode");
            if (info->HasEvent(CslServerEvents::EVENT_PRIVATE))
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
                m_toolTipTextLeft=info->GetBestDescription();
                m_toolTipTitle=wxT("CSL server notification");

                if (!CslGameConnection::IsPlaying())
                    CslToolTip::Init(this, 0, true);

                CslTTS::Say(m_toolTipTitle+wxT(". ")+m_toolTipTextLeft+wxT(" ")+m_toolTipTextRight);

                RequestUserAttention();
            }
#undef TT_SPACER
        }
        return;
    }

    if (m_extendedDlg->GetInfo()==info && m_extendedDlg->IsShown())
    {
        switch (type)
        {
            case CslPongEvent::PLAYERSTATS:
                m_extendedDlg->UpdatePlayerData();
                break;

            case CslPongEvent::TEAMSTATS:
                m_extendedDlg->UpdateTeamData();
                break;

            default:
                break;
        }
    }

    if (type==CslPongEvent::PLAYERSTATS)
    {
        if (pane_country->GetMode()==CslPanelCountry::MODE_PLAYER_SINGLE)
        {
            if (info==m_oldSelectedInfo)
            {
                pane_country->Reset(CslPanelCountry::MODE_PLAYER_SINGLE);
                pane_country->UpdateData(info);
            }
        }
        else if (pane_country->GetMode()==CslPanelCountry::MODE_PLAYER_MULTI)
        {
            loopv(m_countryServers)
            {
                if (m_countryServers[i]!=info)
                    continue;

                if (info->Search)
                {
                    info->Search=false;
                    info->PingExt(false);
                    pane_country->UpdateData(info);
                }

                m_countryServers.RemoveAt(+i);
                break;
            }
        }

        loopv(m_serverViews)
        {
            if (m_serverViews[i]->GetServerInfo()==info)
                m_serverViews[i]->UpdateData();
        }

        if (m_searchedServers.size() && pane_search->IsPlayerSearch())
        {
            bool found = false;

            wxInt32 searchResultPlayerOld = m_searchResultPlayer,
                    searchResultServerOld = m_searchResultServer;

            loopv(m_searchedServers)
            {
                if (m_searchedServers[i]!=info)
                    continue;
                if (info->Search)
                {
                    info->Search=false;
                    info->PingExt(false);

                    pane_search->IncrementProgress();

                    CslPlayerStats& stats=m_searchedServers[i]->PlayerStats;
                    loopvj(stats.m_stats)
                    {
                        CslPlayerStatsData *data=stats.m_stats[j];
                        if (data->Ok && data->Name.Lower().Contains(m_searchString))
                        {
                            list_ctrl_player_search->AddResult(info, data);
                            if (++m_searchResultPlayer==1)
                                ToggleAuiPane(MENU_VIEW_PLAYER_SEARCH, true);
                            found=true;
                        }
                    }

                    if (found)
                    {
                        list_ctrl_master->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER, true, true, info);
                        list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER, true, true, info);
                        m_searchResultServer++;
                    }
                    else
                    {
                        list_ctrl_master->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER, false, false, info);
                        list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER, false, false, info);
                    }
                }

                break;
            }


            if (m_searchResultPlayer!=searchResultPlayerOld ||
                m_searchResultServer!=searchResultServerOld)
                pane_search->SetResult(wxString::Format(_("%d players on %d servers"),
                                             m_searchResultPlayer, m_searchResultServer));
        }
    }
}

void CslFrame::OnTimer(wxTimerEvent& event)
{
    bool playing=CslGameConnection::IsPlaying();
    bool waiting=CslGameConnection::IsWaiting();

    if (!(CslGetSettings().DontUpdatePlaying && playing))
    {
        CslGame *game=TreeGamesGetSelectedGame();
        CslMaster *master=TreeGamesGetSelectedMaster();

        if (m_timerCount%m_timerUpdate==0)
        {
            if (game)
            {
                CslArrayCslServerInfo servers;
                m_engine->GetFavourites(servers);

                list_ctrl_favourites->UpdateServers(servers);
                list_ctrl_master->UpdateServers(master? master->GetServers() : game->GetServers());

                wxTreeItemId item = tree_ctrl_games->GetSelection();

                if (item.IsOk())
                {
                    wxString s = master ? master->GetConnection().Address : game->GetName();
                    s << wxString::Format(_(" (%d Players)"), list_ctrl_master->GetPlayerCount());

                    if (tree_ctrl_games->GetItemText(item)!=s)
                        tree_ctrl_games->SetItemText(item,s);
                }
            }

            if (CslGetSettings().SearchLAN)
            {
                CslArrayCslGame& games=m_engine->GetGames();
                loopv(games) m_engine->BroadcastPing(games[i]);
            }

            loopv(m_serverViews)
                m_serverViews[i]->CheckServerStatus();
        }

        m_timerCount++;

        bool green = (game && m_engine->PingServers(game, m_timerInit)>0);

        if (green)
            m_engine->PingServersEx();

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

        pane_traffic->UpdateStats();
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
        static wxUint32 lightCount=0;

        if (++lightCount%4==0)
            CslStatusBar::Light(LIGHT_GREY);
    }
}

void CslFrame::OnListItemSelected(wxListEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    if (!info)
        return;

    list_ctrl_info->UpdateServer(info);

    if (m_oldSelectedInfo)
        m_oldSelectedInfo->PingExt(false);
    m_oldSelectedInfo=info;

    pane_country->Reset(CslPanelCountry::MODE_PLAYER_SINGLE);
    CountryViewSetCaption(info);

    player_info->SetServerInfo(info);

    info->PingExt(true);

    if (!m_engine->PingEx(info))
    {
        player_info->UpdateData();
        pane_country->UpdateData(info);
    }

    {
        wxAuiPaneInfo& pane=m_aui.GetPane(list_ctrl_info);
        if (pane.IsOk())
            pane.Caption(wxString(CSL_CAPTION_SERVER_INFO)+wxT(": ")+info->GetBestDescription());
    }
    {
        wxAuiPaneInfo& pane=m_aui.GetPane(player_info);
        if (pane.IsOk())
            pane.Caption(ServerViewGetCaption(info,true));
    }
    m_aui.Update();
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

    wxWindowUpdateLocker lock(this);

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

    CslArrayCslServerInfo *servers;

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
        CslGetSettings().LastGame=game->GetName();

    list_ctrl_master->Clear();
    SetTotalPlaytime(game);

    list_ctrl_master->SetMasterSelected(master!=NULL);
    list_ctrl_master->UpdateServers(*servers);

    m_timerCount=0;
    m_timerInit=true;
}

void CslFrame::OnTreeRightClick(wxTreeEvent& event)
{
    const wxTreeItemId& item = event.GetItem();

    // the "right-clicked" item is the selected one?
    // change the selection if necessary

    wxTreeItemId old = tree_ctrl_games->GetSelection();

    if (!old.IsOk() || item!=old)
    {
        wxWindowUpdateLocker lock(this);
        tree_ctrl_games->SelectItem(item);
    }

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
#ifndef __WXMAC__
        case wxID_ICONIZE_FRAME:
        case wxID_RESTORE_FRAME:
            ToggleShow();
            break;
#endif
        case wxID_EXIT:
            ::wxGetApp().Shutdown(CslApp::CSL_SHUTDOWN_NORMAL);
            Close(false);
            break;

        case wxID_PREFERENCES:
        {
            CslValueRestore<bool> shown(g_csl_settings_shown, true);

            CslDlgSettings dlg(this);

            if (dlg.ShowModal()!=wxID_OK)
                break;

            m_timerUpdate = CslGetSettings().UpdateInterval/CSL_TIMER_SHOT;
            m_engine->SetUpdateInterval(CslGetSettings().UpdateInterval);

            CslSettings::SaveSettings();
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
                    list_ctrl_favourites->UpdateServer(info);
                    list_ctrl_favourites->ListSort();
                }
            }
            else
            {
                wxUint32 fourcc=TreeGamesGetSelectedGame()->GetFourCC();
                CslDlgAddMaster *dlg = new CslDlgAddMaster(this);
                dlg->InitDlg(&fourcc);

                if (dlg->ShowModal()==wxID_OK)
                {
                    CslGame *game=m_engine->GetGames()[fourcc];
                    CslMaster *master=game->GetMasters().Last();
                    game->AddMaster(master);
                    TreeGamesAddMaster(wxTreeItemId(),master,true);
                }
            }
            break;
        }

        case MENU_DEL:
        {
            if (event.GetEventObject()==menuMaster)
            {
                list_ctrl_master->Clear();
                TreeGamesRemoveMaster();
            }
            else
            {
                // FIXME let the engine send a delete event and adjust stuff
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

                if (list_ctrl_info->GetServerInfo()==info)
                    list_ctrl_info->UpdateServer(NULL);

                list_ctrl_player_search->RemoveServer(info);

                loopvrev(m_serverViews)
                {
                    if (m_serverViews[i]->GetServerInfo()==info)
                    {
                        if (i==0)
                        {
                            m_serverViews[i]->SetServerInfo(NULL);
                            m_serverViews[i]->UpdateData();
                            wxAuiPaneInfo& pane=m_aui.GetPane(m_serverViews[i]);
                            if (pane.IsOk())
                            {
                                pane.Caption(ServerViewGetCaption(NULL,true));
                                m_aui.Update();
                            }
                        }
                        else
                        {
                            if (m_aui.DetachPane(m_serverViews[i]))
                                m_aui.Update();

                            delete m_serverViews[i];
                            m_serverViews.RemoveAt(+i);
                        }
                        info->PingExt(false);
                    }
                }

                m_engine->DeleteServer(&info->GetGame(), info);
            }
            break;
        }

        case MENU_UPDATE:
            UpdateMaster();
            break;

        case MENU_GAME_SERVER_COUNTRY:
        {
            m_countryServers.Empty();

            CslGame *game=TreeGamesGetSelectedGame();
            CslMaster *master=TreeGamesGetSelectedMaster();

            if (game)
            {
                const CslArrayCslServerInfo& servers=master ? master->GetServers() : game->GetServers();

                pane_country->Reset(CslPanelCountry::MODE_SERVER,m_countryServers.size());
                CountryViewSetCaption(NULL);
                ToggleAuiPane(MENU_VIEW_COUNTRY, true);

                loopv(servers)
                {
                    if (CslEngine::PingOk(*servers[i], CslGetSettings().UpdateInterval))
                        pane_country->UpdateData(servers[i]);
                }

            }

            break;
        }

        case MENU_GAME_PLAYER_COUNTRY:
        {
            loopv(m_countryServers) m_countryServers[i]->PingExt(false);
            m_countryServers.Empty();

            CslGame *game=TreeGamesGetSelectedGame();
            CslMaster *master=TreeGamesGetSelectedMaster();

            if (game)
            {
                const CslArrayCslServerInfo& servers=master ? master->GetServers() : game->GetServers();

                loopv(servers)
                {
                    if (CslEngine::PingOk(*servers[i], CslGetSettings().UpdateInterval) &&
                        servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
                    {
                        m_countryServers.push_back(servers[i]);
                        servers[i]->Search=true;
                        servers[i]->PingExt(true);
                    }
                }

                pane_country->Reset(CslPanelCountry::MODE_PLAYER_MULTI,m_countryServers.size());
                CountryViewSetCaption(NULL);
                ToggleAuiPane(MENU_VIEW_COUNTRY, true);
            }

            break;
        }

        case MENU_VIEW_GAMES:
        case MENU_VIEW_SERVER_INFO:
        case MENU_VIEW_PLAYER_LIST:
        case MENU_VIEW_TRAFFIC:
        case MENU_VIEW_PLAYER_SEARCH:
        case MENU_VIEW_COUNTRY:
        case MENU_VIEW_FAVOURITES:
        case MENU_VIEW_SEARCH:
            ToggleAuiPane(event.GetId());
            break;

        case MENU_VIEW_OUTPUT:
            m_outputDlg->Show(!m_outputDlg->IsShown());
            break;

        case MENU_VIEW_AUTO_SORT:
            CslGetSettings().AutoSortColumns=event.IsChecked();
            list_ctrl_master->ListSort();
            list_ctrl_favourites->ListSort();
            break;

        case MENU_VIEW_RELAYOUT:
            m_aui.LoadPerspective(m_aui_default_layout);
            SetAuiPaneSettings();
            m_aui.Update();
            break;

        case wxID_ABOUT:
        {
            CslValueRestore<bool> shown(g_csl_about_shown, true);
            CslDlgAbout *dlg = new CslDlgAbout(this);
            dlg->ShowModal();
            break;
        }

        case SEARCH_TEXT:
        {
            wxString s = pane_search->GetSearchText();

            if (s.IsEmpty())
            {
                list_ctrl_master->SearchServers();
                list_ctrl_favourites->SearchServers();
            }

            if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED && pane_search->IsServerSearch())
            {
                wxInt32 c = list_ctrl_master->SearchServers(s) +
                            list_ctrl_favourites->SearchServers(s);

                pane_search->SetSearchCtrlError(!(c || s.IsEmpty()));

                if (!s.IsEmpty())
                    pane_search->SetResult(wxString::Format(_("%d servers"), c));
            }
            else if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;
        }

        case SEARCH_BUTTON_SEARCH:
        {
            if (!pane_search->IsPlayerSearch())
                break;

            m_searchString = pane_search->GetSearchText().Lower();
            m_searchResultPlayer=m_searchResultServer=0;

            pane_search->SetProgress(0);
            pane_search->SetResult(wxString::Format(_("%d players on %d servers"), 0, 0));

            list_ctrl_master->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER,false,true);
            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_FOUND_PLAYER,false,true);
            list_ctrl_player_search->ListClear();

            if (m_searchString.IsEmpty())
                break;

            CslGame *game=TreeGamesGetSelectedGame();

            if (!game)
                break;

            {
                loopv(m_searchedServers)
                {
                    CslServerInfo *info = m_searchedServers[i];
                    if (info->Search)
                    {
                        info->Search = false;
                        info->PingExt(false);
                    }
                }

                m_searchedServers.Empty();

                const CslArrayCslServerInfo& servers=game->GetServers();
                loopv(servers)
                {
                    if (!servers[i]->IsFavourite() &&
                        CslEngine::PingOk(*servers[i], CslGetSettings().UpdateInterval) &&
                        servers[i]->ExtInfoStatus==CSL_EXT_STATUS_OK)
                    {
                        m_searchedServers.push_back(servers[i]);
                        servers[i]->Search=true;
                        servers[i]->PingExt(true);
                        list_ctrl_master->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,true,false,servers[i]);
                    }
                }
            }

            {
                CslArrayCslServerInfo servers;
                m_engine->GetFavourites(servers);
                loopv(servers)
                {
                    CslServerInfo *server=servers[i];

                    if (CslEngine::PingOk(*server, CslGetSettings().UpdateInterval) &&
                        server->ExtInfoStatus==CSL_EXT_STATUS_OK)
                    {
                        m_searchedServers.push_back(server);
                        server->Search=true;
                        server->PingExt(true);
                        list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_SEARCH_PLAYER,true,false,server);
                    }
                }
            }

            pane_search->SetProgressRange(m_searchedServers.size());

            m_engine->PingServersEx(true);
            break;
        }

        case MENU_GAME_SEARCH_LAN:
            CslGetSettings().SearchLAN=event.IsChecked();
            break;

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
                    CreateServerView(info,event.GetId()-MENU_SERVER_EXT_MICRO);
            }
            else if (CSL_MENU_EVENT_IS_SRVMSG(id))
            {
                CslServerInfo *info=(CslServerInfo*)event.GetClientData();
                CslDlgGeneric *dlg=new CslDlgGeneric(this,CSL_DLG_GENERIC_RESIZE|CSL_DLG_GENERIC_TEXT|CSL_DLG_GENERIC_CLOSE,
                                                     _("Server message: ")+info->GetBestDescription(),info->InfoText,
                                                     wxArtProvider::GetBitmap(wxART_INFORMATION,wxART_CMN_DIALOG));
                dlg->Show();

            }
            else if (CSL_MENU_EVENT_IS_URICOPY(id))
            {
                wxUint32 i;
                wxString s;
                CslServerInfo *info;
                bool con,add,pass=false;
                wxArrayPtrVoid *servers=(wxArrayPtrVoid*)event.GetClientData();

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
                        s<<CSL_NEWLINE_WX;

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
                ServerBrowserSetCaption((wxInt32)(long)event.GetClientData());
    }
}

void CslFrame::OnKeypress(wxKeyEvent& event)
{
    static wxUint32 lastTicks=0;
    wxUint32 ticks = GetTicks();

    if (ticks-lastTicks<200)
    {
        event.Skip();
        return;
    }

    lastTicks = ticks;

    if (event.GetKeyCode()==WXK_ESCAPE)
    {
        if (CslGameConnection::IsWaiting())
        {
            CslServerInfo *info=CslGameConnection::GetInfo();

            list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
            list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);

            CslGameConnection::Reset();

            return;
        }
    }

    event.Skip();
}

void CslFrame::OnSize(wxSizeEvent& event)
{
    static bool init=true;

    if (!IsMaximized() && event.GetSize()!=CslGetSettings().FrameSizeMax)
    {
        if (init)
        {
            SetMinSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT));
            init=false;
        }
        CslGetSettings().FrameSize=event.GetSize();
        CslGetSettings().FrameSizeMax=wxDefaultSize;
    }
    else
        CslGetSettings().FrameSizeMax=event.GetSize();

    event.Skip();
}

#ifndef __WXMAC__
void CslFrame::OnIconize(wxIconizeEvent& event)
{
    if (CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY) && IsIconized())
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
        CslToolTip::Reset();

    event.Skip();
}

void CslFrame::OnClose(wxCloseEvent& event)
{
    static bool closing = false;

    if (closing)
        return;

    CslValueRestore<bool> closing_restore(closing, 1);

    if (event.GetEventObject()==m_outputDlg)
    {
        CslMenu::CheckItem(MENU_VIEW_OUTPUT,false);
        return;
    }
    else if (event.GetEventObject()!=this)
        return;

    if (::wxGetApp().Shutdown()!=CslApp::CSL_SHUTDOWN_FORCE)
    {
#ifndef __WXMAC__
        if (CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_USE_SYSTRAY) &&
            CSL_FLAG_CHECK(CslGetSettings().Systray, CSL_SYSTRAY_CLOSE) &&
            ::wxGetApp().Shutdown()!=CslApp::CSL_SHUTDOWN_NORMAL)
        {
            ToggleShow(-1);
            return;
        }
        else
#endif
            if (CslGameConnection::IsPlaying())
            {
                if (wxMessageBox(    _("There is currently a game running.\n"
                                 wxT_2("Detach the game process and continue?")),
                                 _("Game running"), wxYES_NO|wxICON_QUESTION, this)==wxYES)
                    CslGameConnection::Detach();
                else
                    return;
            }
    }
    else if (CslGameConnection::IsPlaying()) // the game should handle it itself :)
        CslGameConnection::Detach();


    CSL_LOG_DEBUG("closing\n");

    // close all open dialogs first
    event.SetCanVeto(false);

    CslArraywxWindow dialogs;

    if (GetChildWindowsByClassInfo(this, CLASSINFO(wxDialog), dialogs))
    {
        loopv(dialogs)
            ((wxDialog*)dialogs[i])->GetEventHandler()->ProcessEvent(event);
    }

    event.Skip();
}

void CslFrame::OnAuiPaneClose(wxAuiManagerEvent& event)
{
    CslServerInfo *info;

    if (event.pane->name.StartsWith(wxT("players")))
    {
        loopv(m_serverViews)
        {
            if (m_serverViews[i]==event.pane->window)
            {
                if ((info=m_serverViews[i]->GetServerInfo())!=NULL)
                    info->PingExt(false);
                if (i==0)
                {
                    CslMenu::CheckItem(MENU_VIEW_PLAYER_LIST,false);
                    m_serverViews[i]->SetServerInfo(NULL);
                }
                else
                    m_serverViews.RemoveAt(i); // window gets destroyed by aui manager

                return;
            }
        }

        return;
    }

    if (event.pane->name==wxT("games"))
        CslMenu::CheckItem(MENU_VIEW_GAMES, false);
    else if (event.pane->name==wxT("info"))
        CslMenu::CheckItem(MENU_VIEW_SERVER_INFO, false);
    else if (event.pane->name==wxT("playerlist"))
        CslMenu::CheckItem(MENU_VIEW_PLAYER_LIST, false);
    else if (event.pane->name==wxT("traffic"))
        CslMenu::CheckItem(MENU_VIEW_TRAFFIC, false);
    else if (event.pane->name==wxT("search"))
        CslMenu::CheckItem(MENU_VIEW_PLAYER_SEARCH, false);
    else if (event.pane->name==wxT("country"))
        CslMenu::CheckItem(MENU_VIEW_COUNTRY, false);
    else if (event.pane->name==wxT("favlist"))
        CslMenu::CheckItem(MENU_VIEW_FAVOURITES, false);

    event.pane->window->Hide();
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
    const wxString& version = event.GetString();

    if (version.Cmp(CSL_VERSION_STR)>0)
    {
        CslDlgGeneric *dlg = new CslDlgGeneric(this,CSL_DLG_GENERIC_URL|CSL_DLG_GENERIC_CLOSE,
                                               wxString::Format(    _("New version %s"), version.c_str()),
                                               wxString::Format(    _("There is a new version (%s)\n"\
                                                                wxT_2("of Cube Server Lister available.\n")),
                                                                version.c_str()),
                                               wxArtProvider::GetBitmap(wxART_INFORMATION ,wxART_CMN_DIALOG),
                                               CSL_WEBADDR_STR);
        dlg->Show();
    }

    if (m_versionCheckThread)
    {
        delete m_versionCheckThread;
        m_versionCheckThread = NULL;
    }
}

void CslFrame::OnEndProcess(wxCommandEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    CslGameConnection::Reset();

    list_ctrl_master->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
    list_ctrl_favourites->Highlight(CSL_HIGHLIGHT_LOCKED,false,false,info);
    list_ctrl_info->UpdateServer(info);
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
                CSL_LOG_DEBUG("IPC request: %s\n",U2C(event.Request));

                if (event.Request.Cmp(wxT("show"))==0)
                    ToggleShow(1);
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

                        if (!(info=game->FindServerByAddr(CslIPV4Addr(addr))))
                        {
                            info = CslServerInfo::Create(game, host, port, iport);
                            info->View=CslServerInfo::CSL_VIEW_DEFAULT;

                            if (!m_engine->AddServer(game, info))
                            {
                                CslServerInfo::Destroy(info);
                                break;
                            }
                        }
                        info->Password = pass;

                        if (addfavourites)
                        {
                            if (!info->IsFavourite())
                            {
                                info->SetFavourite();
                                list_ctrl_favourites->UpdateServer(info);
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
