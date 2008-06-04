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
#include <wx/stdpaths.h>
#include <wx/artprov.h>
#include <wx/sysopt.h>
#include <wx/protocol/http.h>
#include "CslFrame.h"
#include "CslDlgAbout.h"
#include "CslDlgAddMaster.h"
#include "CslDlgGeneric.h"
#include "CslGeoIP.h"
#include "CslTools.h"
#include "CslVersion.h"
#include "csl_icon_png.h"

#ifndef __WXMSW__
#include "img/sb_24.xpm"
#include "img/bf_24.xpm"
#include "img/ac_24.xpm"
#include "img/cb_24.xpm"
#include "img/master_24.xpm"
#include "img/close_14.xpm"
#include "img/close_high_14.xpm"
#include "img/close_press_14.xpm"
#endif

#define CSL_TIMER_SHOT 500

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK,wxID_ANY)
END_DECLARE_EVENT_TYPES()

#define CSL_EVT_VERSIONCHECK(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_VERSIONCHECK,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),

DEFINE_EVENT_TYPE(wxCSL_EVT_VERSIONCHECK)


enum
{
    CSL_BUTTON_SEARCH_CLOSE = MENU_END + 1,
    CSL_TEXT_SEARCH,

    CSL_CHECK_FILTER_OFFLINE_MASTER,
    CSL_CHECK_FILTER_FULL_MASTER,
    CSL_CHECK_FILTER_EMPTY_MASTER,
    CSL_CHECK_FILTER_NONEMPTY_MASTER,
    CSL_CHECK_FILTER_MM2_MASTER,
    CSL_CHECK_FILTER_MM3_MASTER,
    CSL_CHECK_FILTER_OFFLINE_FAVOURITES,
    CSL_CHECK_FILTER_FULL_FAVOURITES,
    CSL_CHECK_FILTER_EMPTY_FAVOURITES,
    CSL_CHECK_FILTER_NONEMPTY_FAVOURITES,
    CSL_CHECK_FILTER_MM2_FAVOURITES,
    CSL_CHECK_FILTER_MM3_FAVOURITES,
    CSL_BUTTON_FILTER_RESET,

    CSL_SPLITTER_LISTS
};

enum
{
    IMG_LIST_TREE_MASTER = 0,
    IMG_LIST_TREE_SB,
    IMG_LIST_TREE_AC,
    IMG_LIST_TREE_BF,
    IMG_LIST_TREE_CB
};


BEGIN_EVENT_TABLE(CslFrame, wxFrame)
    EVT_SIZE(CslFrame::OnSize)
    CSL_EVT_PONG(wxID_ANY,CslFrame::OnPong)
    EVT_TIMER(wxID_ANY,CslFrame::OnTimer)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslFrame::OnListItemSelected)
    EVT_TREE_SEL_CHANGED(wxID_ANY,CslFrame::OnTreeLeftClick)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,CslFrame::OnTreeRightClick)
    EVT_MENU(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_TEXT(CSL_TEXT_SEARCH,CslFrame::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslFrame::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslFrame::OnCommandEvent)
    // dont use wxID_ANY for EVT_BUTTON, because on wxMAC
    // wxID_CANCEL is sent when pressing ESC
    EVT_BUTTON(CSL_BUTTON_SEARCH_CLOSE,CslFrame::OnCommandEvent)
    EVT_BUTTON(CSL_BUTTON_FILTER_RESET,CslFrame::OnCommandEvent)
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
    m_AuiMgr.SetManagedWindow(this);

    LoadSettings();

    long listctlStyle=wxLC_REPORT;
    long treectlStyle=wxTR_HIDE_ROOT|wxTR_DEFAULT_STYLE;

#undef wxLC_REPORT
#undef wxTR_DEFAULT_STYLE
#ifdef __WXMSW__
    listctlStyle|=wxNO_BORDER;
    treectlStyle|=wxNO_BORDER;
#else
    listctlStyle|=wxSUNKEN_BORDER;
    treectlStyle|=wxSUNKEN_BORDER;
#endif // __WXMSW__
#define wxLC_REPORT listctlStyle
#define wxTR_DEFAULT_STYLE treectlStyle

#ifdef __WXMSW__
    m_imgListTree.Create(24,24,true);
    m_imgListTree.Add(wxICON(master_24));
    m_imgListTree.Add(wxICON(sb_24));
    m_imgListTree.Add(wxICON(ac_24));
    m_imgListTree.Add(wxICON(bf_24));
    m_imgListTree.Add(wxICON(cb_24));
#else
    m_imgListTree.Create(24,24,true);
    m_imgListTree.Add(wxBitmap(master_24_xpm));
    m_imgListTree.Add(wxBitmap(sb_24_xpm));
    m_imgListTree.Add(wxBitmap(ac_24_xpm));
    m_imgListTree.Add(wxBitmap(bf_24_xpm));
    m_imgListTree.Add(wxBitmap(cb_24_xpm));
#endif

    m_imgListButton.Create(14,14,true);
#ifdef __WXMSW__
    m_imgListButton.Add(wxICON(close_14));
    m_imgListButton.Add(wxICON(close_high_14));
    m_imgListButton.Add(wxICON(close_press_14));
#else
    m_imgListButton.Add(wxBitmap(close_14_xpm));
    m_imgListButton.Add(wxBitmap(close_high_14_xpm));
    m_imgListButton.Add(wxBitmap(close_press_14_xpm));
#endif
    m_engine=new CslEngine(this);

    m_outputDlg=new CslDlgOutput(this);
    m_extendedDlg=new CslDlgExtended(this);
    m_trafficDlg=NULL;

    pane_favourites=new wxPanel(this, wxID_ANY);
    pane_master=new wxPanel(this, wxID_ANY);
    panel_search=new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);
    tree_ctrl_games=new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_NO_LINES|wxTR_LINES_AT_ROOT|wxTR_DEFAULT_STYLE);
    list_ctrl_info=new CslListCtrlInfo(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER);
    list_ctrl_players=new CslListCtrlPlayer(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
    text_ctrl_search=new wxTextCtrl(panel_search, CSL_TEXT_SEARCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_RICH2);
    list_ctrl_master=new CslListCtrlServer(pane_master, CSL_LIST_MASTER, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    checkbox_filter_offline_master=new wxCheckBox(pane_master, CSL_CHECK_FILTER_OFFLINE_MASTER, _("&Offline"));
    checkbox_filter_full_master=new wxCheckBox(pane_master, CSL_CHECK_FILTER_FULL_MASTER, _("F&ull"));
    checkbox_filter_empty_master=new wxCheckBox(pane_master, CSL_CHECK_FILTER_EMPTY_MASTER, _("&Empty"));
    checkbox_filter_nonempty_master=new wxCheckBox(pane_master, CSL_CHECK_FILTER_NONEMPTY_MASTER, _("&Not empty"));
    checkbox_filter_mm2_master=new wxCheckBox(pane_master, CSL_CHECK_FILTER_MM2_MASTER, _("Mastermode &2"));
    checkbox_filter_mm3_master=new wxCheckBox(pane_master, CSL_CHECK_FILTER_MM3_MASTER, _("Mastermode &3"));
    list_ctrl_favourites=new CslListCtrlServer(pane_favourites, CSL_LIST_FAVOURITE, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    checkbox_filter_offline_favourites=new wxCheckBox(pane_favourites, CSL_CHECK_FILTER_OFFLINE_FAVOURITES, _("&Offline"));
    checkbox_filter_full_favourites=new wxCheckBox(pane_favourites, CSL_CHECK_FILTER_FULL_FAVOURITES, _("F&ull"));
    checkbox_filter_empty_favourites=new wxCheckBox(pane_favourites, CSL_CHECK_FILTER_EMPTY_FAVOURITES, _("&Empty"));
    checkbox_filter_nonempty_favourites=new wxCheckBox(pane_favourites, CSL_CHECK_FILTER_NONEMPTY_FAVOURITES, _("&Not empty"));
    checkbox_filter_mm2_favourites=new wxCheckBox(pane_favourites, CSL_CHECK_FILTER_MM2_FAVOURITES, _("Mastermode &2"));
    checkbox_filter_mm3_favourites=new wxCheckBox(pane_favourites, CSL_CHECK_FILTER_MM3_FAVOURITES, _("Mastermode &3"));

    set_properties();
    do_layout();
    // end wxGlade

#ifdef __WXMSW__  // key events from childs doesn't send to its parent
    list_ctrl_master->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    list_ctrl_favourites->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    list_ctrl_info->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    tree_ctrl_games->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
    text_ctrl_search->Connect(wxEVT_CHAR,wxKeyEventHandler(CslFrame::OnKeypress),NULL,this);
#endif

    tree_ctrl_games->SetImageList(&m_imgListTree);
    m_treeGamesRoot=tree_ctrl_games->AddRoot(wxEmptyString,-1,-1);

    m_timerUpdate=g_cslSettings->m_updateInterval/CSL_TIMER_SHOT;
    m_timerCount=0;
    m_lightCount=0;
    m_timer.SetOwner(this);

    if (m_engine->InitEngine(g_cslSettings->m_updateInterval))
    {
        CslGame *game=NULL;
        CslMaster *master;

        if (!LoadServers())
        {
            master=new CslMaster(CSL_GAME_SB,CslMasterConnection(CSL_DEFAULT_MASTER_SB,CSL_DEFAULT_MASTER_PATH_SB));
            game=m_engine->AddMaster(master);
            m_engine->SetCurrentGame(game,NULL);
            TreeAddGame(game,true);

            master=new CslMaster(CSL_GAME_AC,CslMasterConnection(CSL_DEFAULT_MASTER_AC,CSL_DEFAULT_MASTER_PATH_AC));
            game=m_engine->AddMaster(master);
            TreeAddGame(game);

            master=new CslMaster(CSL_GAME_BF,CslMasterConnection(CSL_DEFAULT_MASTER_BF,CSL_DEFAULT_MASTER_PORT_BF));
            game=m_engine->AddMaster(master);
            TreeAddGame(game);

            master=new CslMaster(CSL_GAME_CB,CslMasterConnection(CSL_DEFAULT_MASTER_CB,CSL_DEFAULT_MASTER_PATH_CB));
            game=m_engine->AddMaster(master);
            TreeAddGame(game);

            CslServerInfo *info;
            info=new CslServerInfo();
            info->CreateFavourite(CSL_GAME_SB,CSL_DEFAULT_SERVER_ADDR_SB1);
            m_engine->AddServer(info,-1);
            info=new CslServerInfo();
            info->CreateFavourite(CSL_GAME_SB,CSL_DEFAULT_SERVER_ADDR_SB2);
            m_engine->AddServer(info,-1);
        }
        else
        {
            vector<CslGame*>*games=m_engine->GetGames();
            loopv(*games)
            {
                game=games->at(i);
                TreeAddGame(game,i==0);
            }
        }

        CslMenu::EnableMenuItem(MENU_MASTER_ADD);

        list_ctrl_master->ListUpdate(m_engine->GetCurrentGame()->GetServers());
        list_ctrl_favourites->ListUpdate(m_engine->GetFavourites());
        //tree_ctrl_games->ExpandAll();

        m_timerInit=true;
        m_timer.Start(CSL_TIMER_SHOT);
    }
    else
    {
        wxMessageBox(_("Failed to initialise internal engine!\n"\
                       "Please try to restart the application."),
                     _("Fatal error!"),wxICON_ERROR,this);
        delete m_engine;
        m_engine=NULL;
    }

    m_extendedDlg->SetEngine(m_engine);

    m_versionCheckThread=new CslVersionCheckThread(this);
    m_versionCheckThread->Run();
}

CslFrame::~CslFrame()
{
    if (m_timer.IsRunning())
        m_timer.Stop();

    if (CslConnectionState::IsPlaying())
    {
        CslServerInfo *info=CslConnectionState::GetInfo();
        if (info)
            CslGame::ConnectCleanupConfig(info->m_type,CslConnectionState::GetConfig());
    }

    tree_ctrl_games->DeleteAllItems();

    if (m_engine)
    {
        SaveServers();
        delete m_engine;
    }

    SaveSettings();
    delete g_cslSettings;

    delete m_menu;

    if (m_versionCheckThread)
    {
        m_versionCheckThread->Delete();
        delete m_versionCheckThread;
    }

    m_AuiMgr.UnInit();
}

void CslFrame::set_properties()
{
    SetTitle(_("Cube Server Lister"));

    CreateMainMenu();
    UpdateFilterCheckBoxes();

    bitmap_button_search_clear=new wxBitmapButton(panel_search,CSL_BUTTON_SEARCH_CLOSE,
            wxNullBitmap,wxDefaultPosition,wxDefaultSize,wxNO_BORDER);
    bitmap_button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(0));
    bitmap_button_search_clear->SetSize(bitmap_button_search_clear->GetBestSize());

    bitmap_button_search_clear->Connect(wxEVT_ENTER_WINDOW,
                                        wxMouseEventHandler(CslFrame::OnMouseEnter),NULL,this);
    bitmap_button_search_clear->Connect(wxEVT_LEAVE_WINDOW,
                                        wxMouseEventHandler(CslFrame::OnMouseLeave),NULL,this);
    bitmap_button_search_clear->Connect(wxEVT_LEFT_DOWN,
                                        wxMouseEventHandler(CslFrame::OnMouseLeftDown),NULL,this);

    list_ctrl_players->ListInit(true);

    SetMinSize(wxSize(CSL_FRAME_MIN_WIDTH,CSL_FRAME_MIN_HEIGHT));

    wxMemoryInputStream stream(csl_icon_png,sizeof(csl_icon_png));
    // see wx_wxbitmap.html
#ifdef __WXMSW__
    SetIcon(wxICON(aa_csl_48));
#else
    wxIcon icon;
    wxImage image(stream,wxBITMAP_TYPE_PNG);
    wxBitmap bitmap(image);
    icon.CopyFromBitmap(bitmap);
    SetIcon(icon);
#endif
}

void CslFrame::do_layout()
{
    wxInt32 i=0;

    CslStatusBar *statusBar=new CslStatusBar(this);
    SetStatusBar(statusBar);
    CslStatusBar::InitBar(statusBar);

    list_ctrl_master->ListInit(m_engine,list_ctrl_info,NULL,list_ctrl_favourites,
                               m_extendedDlg,g_cslSettings->m_filterMaster);
    list_ctrl_favourites->ListInit(m_engine,list_ctrl_info,list_ctrl_master,NULL,
                                   m_extendedDlg,g_cslSettings->m_filterFavourites);

    wxFlexGridSizer* grid_sizer_favourites=new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_filter_favourites=new wxFlexGridSizer(1, 7, 0, 0);
    wxFlexGridSizer* grid_sizer_master=new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_filter_master=new wxFlexGridSizer(1, 7, 0, 0);
    wxFlexGridSizer* grid_sizer_search=new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticText* label_search_static=new wxStaticText(panel_search, wxID_ANY, _("Search:"));
    grid_sizer_search->Add(label_search_static, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_search->Add(text_ctrl_search, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    panel_search->SetSizer(grid_sizer_search);
    grid_sizer_search->AddGrowableCol(2);
    grid_sizer_master->Add(list_ctrl_master, 1, wxEXPAND, 5);
    wxStaticText* label_filter_main=new wxStaticText(pane_master, wxID_ANY, _("Filter out:"));
    grid_sizer_filter_master->Add(label_filter_main, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->Add(checkbox_filter_offline_master, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->Add(checkbox_filter_full_master, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->Add(checkbox_filter_empty_master, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->Add(checkbox_filter_nonempty_master, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->Add(checkbox_filter_mm2_master, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->Add(checkbox_filter_mm3_master, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_master->AddGrowableCol(1);
    grid_sizer_filter_master->AddGrowableCol(2);
    grid_sizer_filter_master->AddGrowableCol(3);
    grid_sizer_filter_master->AddGrowableCol(4);
    grid_sizer_filter_master->AddGrowableCol(5);
    grid_sizer_filter_master->AddGrowableCol(6);
    grid_sizer_master->Add(grid_sizer_filter_master, 1, wxEXPAND, 0);
    pane_master->SetSizer(grid_sizer_master);
    grid_sizer_master->AddGrowableRow(0);
    grid_sizer_master->AddGrowableCol(0);

    grid_sizer_favourites->Add(list_ctrl_favourites, 1, wxEXPAND, 3);
    wxStaticText* label_filter_favoruites=new wxStaticText(pane_favourites, wxID_ANY, _("Filter out:"));
    grid_sizer_filter_favourites->Add(label_filter_favoruites, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->Add(checkbox_filter_offline_favourites, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->Add(checkbox_filter_full_favourites, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->Add(checkbox_filter_empty_favourites, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->Add(checkbox_filter_nonempty_favourites, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->Add(checkbox_filter_mm2_favourites, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->Add(checkbox_filter_mm3_favourites, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter_favourites->AddGrowableCol(1);
    grid_sizer_filter_favourites->AddGrowableCol(2);
    grid_sizer_filter_favourites->AddGrowableCol(3);
    grid_sizer_filter_favourites->AddGrowableCol(4);
    grid_sizer_filter_favourites->AddGrowableCol(5);
    grid_sizer_filter_favourites->AddGrowableCol(6);
    grid_sizer_favourites->Add(grid_sizer_filter_favourites, 1, wxTOP|wxEXPAND, 3);
    pane_favourites->SetSizer(grid_sizer_favourites);
    grid_sizer_favourites->AddGrowableRow(0);
    grid_sizer_favourites->AddGrowableCol(0);

#ifdef __WXMAC__
    wxSizerItem *sitem=grid_sizer_main_left->GetItem(sizer_filter);
    sitem->SetFlag(wxLEFT|wxTOP|wxBOTTOM|wxEXPAND);
    sitem->SetBorder(4);
#endif

#ifdef __WXMSW__
    i=2; //bitmap button offset
#endif
    grid_sizer_search->Insert(0,bitmap_button_search_clear,0,
                              wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL,i);

    m_sizerMaster=grid_sizer_master;
    m_sizerFavourites=grid_sizer_favourites;
    m_sizerSearch=grid_sizer_search;
    m_sizerFilterMaster=grid_sizer_filter_master;
    m_sizerFilterFavourites=grid_sizer_filter_favourites;

    wxSize size=m_sizerFilterMaster->GetMinSize();
    size.y+=list_ctrl_master->GetBestSize().y;
    //pane_master->SetMinSize(size);
    //pane_favourites->SetMinSize(size);

    m_AuiMgr.AddPane(pane_master,wxAuiPaneInfo().Caption(_("Master list servers")).Center().BestSize(size));
    m_AuiMgr.AddPane(pane_favourites,wxAuiPaneInfo().Caption(_("Favourite servers")).Bottom().Row(2).BestSize(size).MinSize(size));
    size=panel_search->GetBestSize();
    m_AuiMgr.AddPane(panel_search,wxAuiPaneInfo().Caption(_("Search")).BestSize(size).MaxSize(-1,size.y).MinSize(-1,size.y).Bottom().Row(1));
    m_AuiMgr.AddPane(tree_ctrl_games,wxAuiPaneInfo().Caption(_("Games")).Left().Layer(1).Position(0));
    m_AuiMgr.AddPane(list_ctrl_info,wxAuiPaneInfo().Caption(_("Server Info")).Left().Layer(1).Position(1).MinSize(250,-1).BestSize(250,-1));
    m_AuiMgr.AddPane(list_ctrl_players,wxAuiPaneInfo().Caption(_("Players")).Left().Layer(1).Position(2));
    m_AuiMgr.Update();

    SetSize(g_cslSettings->m_frameSize);
    CentreOnScreen();

    ToggleFilter();

// only set splitter gravity on wxMAC here - it doesn't
// work under wxGTK and EVT_SHOW is not send on wxMAC
#ifdef __WXMAC__
    splitter_lists->SetSashGravity(0.8f);
#endif
    i=grid_sizer_filter_favourites->GetMinSize().GetWidth();
    //pane_main_right->SetMinSize(wxSize(i,-1));
}

void CslFrame::CreateMainMenu()
{
    wxMenu *menu;

    m_menubar=new wxMenuBar();
    // Do not add the File menu on __wxMAC__, since Preferences and Exit getting moved
    // to the "Mac menu" so the File menu is empty then.
    // Add Prefernces and Exit to any other menu.
#ifndef __WXMAC__
    menu=new wxMenu();
    m_menubar->Append(menu,_("&File"));
    CslMenu::AddItemToMenu(menu,wxID_PREFERENCES,_("&Settings"),wxART_SETTINGS);
    CslMenu::AddItemToMenu(menu,wxID_EXIT,_("&Exit"),wxART_QUIT);
#endif

    menu=new wxMenu();
    CslMenu::AddItemToMenu(menu,MENU_MASTER_UPDATE,_("&Update from master\tF5"),wxART_RELOAD);
    menu->AppendSeparator();
    CslMenu::AddItemToMenu(menu,MENU_MASTER_ADD,_("Add a &new master ..."),wxART_ADD_BOOKMARK);
    CslMenu::AddItemToMenu(menu,MENU_MASTER_DEL,_("&Remove master"),wxART_DEL_BOOKMARK);
    m_menubar->Append(menu,_("&Master"));
    m_menuMaster=menu;
#ifdef __WXMAC__
    CslMenu::AddItemToMenu(menu,wxID_PREFERENCES,_("&Settings"),wxART_SETTINGS);
    CslMenu::AddItemToMenu(menu,wxID_EXIT,_("&Exit"),wxART_QUIT);
#endif

    menu=new wxMenu();
    CslMenu::AddItemToMenu(menu,MENU_VIEW_FILTER,_("Show &filter\tCTRL+F"),
                           wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItemToMenu(menu,MENU_VIEW_OUTPUT,_("Show &game output\tCTRL+O"),
                           wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItemToMenu(menu,MENU_VIEW_AUTO_SORT,_("Sort &lists automatically\tCTRL+L"),
                           wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItemToMenu(menu,MENU_VIEW_AUTO_FIT,_("Fit columns on window &resize"),
                           wxART_NONE,wxITEM_CHECK);
    m_menubar->Append(menu,_("&View"));

    menu=new wxMenu();
    CslMenu::AddItemToMenu(menu,MENU_INFO_TRAFFIC,_("&Traffic statistics"),wxART_NONE);
#ifndef __WXMAC__
    menu->AppendSeparator();
    CslMenu::AddItemToMenu(menu,wxID_ABOUT,_("A&bout"),wxART_ABOUT);
#endif
    m_menubar->Append(menu,_("&Help"));

    SetMenuBar(m_menubar);
    m_menu=new CslMenu(m_menubar);
}

void CslFrame::OnSize(wxSizeEvent& event)
{
    wxObject *object=event.GetEventObject();

    if (object==list_ctrl_master)
    {
        LOG_DEBUG("master\n");
        return;
    }
    else if (object==list_ctrl_favourites)
    {
        LOG_DEBUG("favourites\n");
        return;
    }
    else
        LOG_DEBUG("other\n");

    event.Skip();
}

void CslFrame::OnPong(wxCommandEvent& event)
{
    wxPostEvent(m_extendedDlg,event);
}

void CslFrame::OnTimer(wxTimerEvent& event)
{
    if (!(g_cslSettings->m_dontUpdatePlaying && CslConnectionState::IsPlaying()))
    {
        if (m_engine->PingServers() && !CslConnectionState::IsWaiting())
            CslStatusBar::Light(LIGHT_GREEN);

        if (m_timerCount==m_timerUpdate || (m_timerInit && m_timerCount%2==0))
        {
            if (m_timerCount==m_timerUpdate)
                m_timerInit=false;
            if (!m_timerInit)
                m_timerCount=0;
            else
                m_timerCount++;

            vector<CslServerInfo*> *servers;
            if (m_engine->GetCurrentMaster())
                servers=m_engine->GetCurrentMaster()->GetServers();
            else
                servers=m_engine->GetCurrentGame()->GetServers();
            list_ctrl_master->ListUpdate(servers);
            servers=m_engine->GetFavourites();
            list_ctrl_favourites->ListUpdate(servers);
        }
        else
            m_timerCount++;

        wxPostEvent(m_extendedDlg,event);
        if (m_trafficDlg) wxPostEvent(m_trafficDlg,event);
    }

    /*if (m_statusCount>-1 && m_statusCount==m_timerCount)
    {
        CslStatusBar::SetText(1,wxT(""));
        m_statusCount=-1;
    }*/

    if (CslConnectionState::IsWaiting())
    {
        wxPostEvent(CslConnectionState::GetList(),event);
        CslStatusBar::Light(LIGHT_RED);
    }

    if (CslConnectionState::IsPlaying())
    {
        wxPostEvent(CslConnectionState::GetList(),event);
        CslStatusBar::Light(LIGHT_YELLOW);
    }

    if (CslStatusBar::Light()!=LIGHT_GREY)
    {
        if (m_lightCount==1)
        {
            CslStatusBar::Light(LIGHT_GREY);
            m_lightCount=0;
        }
        else
            m_lightCount++;
    }
}

void CslFrame::OnListItemSelected(wxListEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    if (!info)
        return;

#ifdef CSL_USE_WX_LIST_DESELECT_WORKAROUND
    if (FindFocus()==event.GetEventObject())
#endif
        list_ctrl_info->UpdateInfo(info);

    LOG_DEBUG("%s\n",info->GetBestDescription().c_str());
}

void CslFrame::OnTreeLeftClick(wxTreeEvent& event)
{
    event.Skip();

    wxTreeItemId item=event.GetItem();
    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);

    CslGame *game=data->GetGame();
    CslMaster *master=data->GetMaster();

    vector<CslServerInfo*> *servers;

    if (!game)
    {
        CslMenu::EnableMenuItem(MENU_MASTER_DEL);
        CslMenu::EnableMenuItem(MENU_MASTER_UPDATE);

        servers=master->GetServers();

        // find the game
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
        servers=game->GetServers();
    }

    list_ctrl_master->ListClear();
    TreeCalcTotalPlaytime(game);

    if (!m_engine->SetCurrentGame(game,master))
        return;

    list_ctrl_master->SetMasterSelected(master!=NULL);
    list_ctrl_master->ListUpdate(servers);

    m_timerCount=0;
    m_timerInit=true;
}

void CslFrame::OnTreeRightClick(wxTreeEvent& event)
{
    event.Skip();

    wxTreeItemId item=event.GetItem();
    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);

    CslMaster *master=data->GetMaster();
    TreeCalcTotalPlaytime(data->GetGame());

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

    PopupMenu(m_menuMaster);
}

void CslFrame::OnCommandEvent(wxCommandEvent& event)
{
    wxUint32 id=event.GetId();

    if (id>=CSL_CHECK_FILTER_OFFLINE_MASTER && id<=CSL_CHECK_FILTER_MM3_FAVOURITES)
    {
        HandleFilterEvent(id,event.IsChecked());
        return;
    }

    switch (event.GetId())
    {
        case wxID_EXIT:
            this->Destroy();
            break;

        case wxID_PREFERENCES:
        {
            CslDlgSettings *dlg=new CslDlgSettings((wxWindow*)this);
            if (dlg->ShowModal()!=wxID_OK)
                break;
            m_timerUpdate=g_cslSettings->m_updateInterval/CSL_TIMER_SHOT;
            m_engine->SetUpdateInterval(g_cslSettings->m_updateInterval);
            SaveSettings();
            break;
        }

        case MENU_MASTER_ADD:
        {
            CSL_GAMETYPE type=CSL_GAME_START;
            wxTreeItemId item=tree_ctrl_games->GetSelection();
            if (item.IsOk())
            {
                CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
                if (data->GetMaster())
                    type=data->GetMaster()->GetType();
                else if (data->GetGame())
                    type=data->GetGame()->GetType();
            }

            CslMaster *master=new CslMaster(type);

            CslDlgAddMaster *dlg=new CslDlgAddMaster((wxWindow*)this,master);
            if (dlg->ShowModal()!=wxID_OK)
            {
                delete master;
                break;
            }
            if (m_engine->AddMaster(master))
                TreeAddMaster(wxTreeItemId(),master,true);
            else
            {
                wxMessageBox(_("Master already exists!"),_("Error"),wxICON_ERROR,this);
                delete master;
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

        case MENU_VIEW_FILTER:
            g_cslSettings->m_showFilter=event.IsChecked();
            ToggleFilter();
            break;

        case MENU_VIEW_OUTPUT:
        {
            wxString bla;
            bla=m_AuiMgr.SavePerspective();
            LOG_DEBUG("%s\n",bla.c_str());
            //m_outputDlg->Show(!m_outputDlg->IsShown());
            break;
        }

        case MENU_VIEW_AUTO_SORT:
            g_cslSettings->m_autoSortColumns=event.IsChecked();
            list_ctrl_master->ToggleSortArrow();
            list_ctrl_favourites->ToggleSortArrow();
            break;

        case MENU_VIEW_AUTO_FIT:
            g_cslSettings->m_autoFitColumns=event.IsChecked();
            break;

        case MENU_INFO_TRAFFIC:
        {
            m_trafficDlg=new CslDlgTraffic(this);
            m_trafficDlg->Show();
            break;
        }

        case wxID_ABOUT:
        {
            CslDlgAbout *dlg=new CslDlgAbout(this);
            dlg->Show();
            break;
        }

        case CSL_TEXT_SEARCH:
        {
            wxString s=event.GetString();
            bool sr=list_ctrl_master->ListSearch(s)>0;
            sr|=list_ctrl_favourites->ListSearch(s)>0;

            if (sr || s.IsEmpty())
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
            break;
        }

        default:
            break;
    }
}

void CslFrame::HandleFilterEvent(const wxInt32 id,const bool checked)
{
    CslListCtrlServer *listctrl;
    wxUint32 *filter;
    wxUint32 f=0,sender=id;

    if (id<CSL_CHECK_FILTER_OFFLINE_FAVOURITES)
    {
        // master filter
        listctrl=list_ctrl_master;
        filter=&g_cslSettings->m_filterMaster;
    }
    else
    {
        // favourites filter
        listctrl=list_ctrl_favourites;
        filter=&g_cslSettings->m_filterFavourites;
        sender-=CSL_CHECK_FILTER_OFFLINE_FAVOURITES;
        sender+=CSL_CHECK_FILTER_OFFLINE_MASTER;
    }

    switch (sender)
    {
        case CSL_CHECK_FILTER_OFFLINE_MASTER:
            f=CSL_FILTER_OFFLINE;
        case CSL_CHECK_FILTER_FULL_MASTER:
            if (!f)
                f=CSL_FILTER_FULL;
        case CSL_CHECK_FILTER_EMPTY_MASTER:
            if (!f)
                f=CSL_FILTER_EMPTY;
        case CSL_CHECK_FILTER_NONEMPTY_MASTER:
            if (!f)
                f=CSL_FILTER_NONEMPTY;
        case CSL_CHECK_FILTER_MM2_MASTER:
            if (!f)
                f=CSL_FILTER_MM2;
        case CSL_CHECK_FILTER_MM3_MASTER:
            if (!f)
                f=CSL_FILTER_MM3;
            if (checked)
                *filter|=f;
            else
                *filter&=~f;

            listctrl->ListFilter(*filter);
            break;

            //FIXME
            /*case CSL_BUTTON_FILTER_RESET:
                g_cslSettings->m_filter=0;
                UpdateFilterCheckBoxes();
                list_ctrl_master->ListFilter(0);
                //if (checkbox_filter_favourites->IsChecked())
                //    list_ctrl_favourites->ListFilter(0);
                break;*/

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
            CslConnectionState::Reset();
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
    //splitter_lists->SetSashGravity(0.8f);
    list_ctrl_master->ListAdjustSize(list_ctrl_master->GetClientSize(),true);
    list_ctrl_favourites->ListAdjustSize(list_ctrl_favourites->GetClientSize(),true);
}

void CslFrame::OnClose(wxCloseEvent& event)
{
    if (CslConnectionState::IsPlaying())
    {
        wxMessageBox(_("There is currently a game running.\n"
                       "Quit the game first and try again."),
                     _("Error"),wxOK|wxICON_ERROR,this);
        return;
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

    CslDlgGeneric *dlg=new CslDlgGeneric(this,CSL_DLG_GENERIC_DEFAULT,_("CSL terminating"),
                                         _("Waiting for engine to terminate ..."),
                                         wxArtProvider::GetBitmap(wxART_INFORMATION,wxART_CMN_DIALOG));
    dlg->Show();
    wxYield();

    event.Skip();
}

void CslFrame::OnMouseEnter(wxMouseEvent& event)
{
    event.Skip();

    bitmap_button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(1));
    bitmap_button_search_clear->Refresh();
    // no event id on wxMAC
    /*switch (event.GetId())
    {
        case CSL_BUTTON_SEARCH_CLOSE:
            bitmap_button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(1));
            break;

        default:
            break;
    }*/
}

void CslFrame::OnMouseLeave(wxMouseEvent& event)
{
    event.Skip();

    bitmap_button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(0));
    bitmap_button_search_clear->Refresh();
    // no event id on wxGTK
    /*switch (event.GetId())
    {
        case CSL_BUTTON_SEARCH_CLEAR:
            bitmap_button_search_clear->SetBitmapLabel(close_14_xpm);
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
            bitmap_button_search_clear->SetBitmapLabel(m_imgListButton.GetBitmap(2));
            bitmap_button_search_clear->Refresh();
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

void CslFrame::ToggleFilter()
{
    if (g_cslSettings->m_showFilter && !m_sizerMaster->IsShown(m_sizerFilterMaster))
    {
        m_sizerMaster->Show(m_sizerFilterMaster);
        m_sizerFavourites->Show(m_sizerFilterFavourites);
    }
    else if (!g_cslSettings->m_showFilter && m_sizerMaster->IsShown(m_sizerFilterMaster))
    {
        m_sizerMaster->Hide(m_sizerFilterMaster,true);
        m_sizerFavourites->Hide(m_sizerFilterFavourites,true);
    }
    m_sizerMaster->Layout();
    m_sizerFavourites->Layout();
}

void CslFrame::UpdateFilterCheckBoxes()
{
    wxUint32 filter;

    filter=g_cslSettings->m_filterMaster;
    checkbox_filter_offline_master->SetValue((filter&CSL_FILTER_OFFLINE)>0);
    checkbox_filter_full_master->SetValue((filter&CSL_FILTER_FULL)>0);
    checkbox_filter_empty_master->SetValue((filter&CSL_FILTER_EMPTY)>0);
    checkbox_filter_nonempty_master->SetValue((filter&CSL_FILTER_NONEMPTY)>0);
    checkbox_filter_mm2_master->SetValue((filter&CSL_FILTER_MM2)>0);
    checkbox_filter_mm3_master->SetValue((filter&CSL_FILTER_MM3)>0);

    filter=g_cslSettings->m_filterFavourites;
    checkbox_filter_offline_favourites->SetValue((filter&CSL_FILTER_OFFLINE)>0);
    checkbox_filter_full_favourites->SetValue((filter&CSL_FILTER_FULL)>0);
    checkbox_filter_empty_favourites->SetValue((filter&CSL_FILTER_EMPTY)>0);
    checkbox_filter_nonempty_favourites->SetValue((filter&CSL_FILTER_NONEMPTY)>0);
    checkbox_filter_mm2_favourites->SetValue((filter&CSL_FILTER_MM2)>0);
    checkbox_filter_mm3_favourites->SetValue((filter&CSL_FILTER_MM3)>0);
}

void CslFrame::UpdateMaster()
{
    wxTreeItemId item=tree_ctrl_games->GetSelection();
    if (!item.IsOk())
        return;

    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
    CslMaster *master=data->GetMaster();
    if (!master)
        return;

    SetStatusText(_("Sending request to master: ")+master->GetConnection().GetAddress(),1);
    m_timer.Stop();

    CslMenu::EnableMenuItem(MENU_MASTER_UPDATE,false);
    tree_ctrl_games->Enable(false);

    wxInt32 num=m_engine->UpdateMaster();
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

void CslFrame::TreeAddMaster(wxTreeItemId parent,CslMaster *master,bool activate)
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
    item=tree_ctrl_games->AppendItem(parent,master->GetConnection().GetAddress(),IMG_LIST_TREE_MASTER,-1,data);

    if (activate)
        tree_ctrl_games->SelectItem(item);
}

void CslFrame::TreeRemoveMaster()
{
    wxTreeItemId item=tree_ctrl_games->GetSelection();
    if (!item.IsOk())
        return;

    if (m_timer.IsRunning())
        m_timer.Stop();

    CslTreeItemData *data=(CslTreeItemData*)tree_ctrl_games->GetItemData(item);
    CslMaster *master=data->GetMaster();
    m_engine->DeleteMaster(master);
    tree_ctrl_games->Delete(item);

    m_timer.Start(CSL_TIMER_SHOT);
}

void CslFrame::TreeAddGame(CslGame *game,bool activate)
{
    if (!game)
    {
        wxASSERT(game);
        return;
    }

    CslTreeItemData *data;
    wxTreeItemId item=TreeFindItem(m_treeGamesRoot,game,NULL);
    wxInt32 img=-1;

    if (!item.IsOk())
    {
        switch (game->GetType())
        {
            case CSL_GAME_SB:
                img=IMG_LIST_TREE_SB;
                break;
            case CSL_GAME_AC:
                img=IMG_LIST_TREE_AC;
                break;
            case CSL_GAME_BF:
                img=IMG_LIST_TREE_BF;
                break;
            case CSL_GAME_CB:
                img=IMG_LIST_TREE_CB;
                break;
            default:
                return;
        }
        data=new CslTreeItemData(game);
        item=tree_ctrl_games->AppendItem(m_treeGamesRoot,game->GetName(),img,-1,data);
        if (activate)
            tree_ctrl_games->SelectItem(item);
    }

    CslMaster *master;
    vector<CslMaster*> *masters=game->GetMasters();
    loopv(*masters)
    {
        master=masters->at(i);
        TreeAddMaster(item,master,false);
    }

    tree_ctrl_games->Expand(item);
}

void CslFrame::TreeCalcTotalPlaytime(CslGame* game)
{
    if (!game)
        return;

    wxString s;
    wxUint32 playtime=0;
    CslServerInfo *info=NULL;
    vector<CslServerInfo*> *servers=game->GetServers();

    loopv(*servers)
    {
        if (info)
        {
            if (info->m_playTimeTotal<servers->at(i)->m_playTimeTotal)
                info=servers->at(i);
        }
        else
            info=servers->at(i);

        playtime+=servers->at(i)->m_playTimeTotal;
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
                                FormatSeconds(info->m_playTimeTotal).c_str());
        }
    }

    CslStatusBar::SetText(1,s);
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
    if (config.Read(wxT("SizeX"),&val)) g_cslSettings->m_frameSize.SetWidth(val);
    if (config.Read(wxT("SizeY"),&val)) g_cslSettings->m_frameSize.SetHeight(val);
    if (config.Read(wxT("UpdateInterval"),&val))
    {
        if (val<CSL_UPDATE_INTERVAL_MIN || val>CSL_UPDATE_INTERVAL_MAX)
            val=CSL_UPDATE_INTERVAL_MIN;
        g_cslSettings->m_updateInterval=val;
    }
    if (config.Read(wxT("NoUpdatePlaying"),&val)) g_cslSettings->m_dontUpdatePlaying=val>0;
    if (config.Read(wxT("ShowFilter"),&val)) g_cslSettings->m_showFilter=val>0;
    if (config.Read(wxT("FilterMaster"),&val)) g_cslSettings->m_filterMaster=val;
    if (config.Read(wxT("FilterFavourites"),&val)) g_cslSettings->m_filterFavourites=val;
    if (config.Read(wxT("WaitOnFullServer"),&val))
    {
        if (val<CSL_WAIT_SERVER_FULL_MIN || val>CSL_WAIT_SERVER_FULL_MAX)
            val=CSL_WAIT_SERVER_FULL_STD;
        g_cslSettings->m_waitServerFull=val;
    }
    if (config.Read(wxT("PingGood"),&val))
    {
        g_cslSettings->m_ping_good=val;
        if (g_cslSettings->m_ping_good>9999)
            g_cslSettings->m_ping_good=CSL_PING_GOOD_STD;
    }
    if (config.Read(wxT("PingBad"),&val))
    {
        g_cslSettings->m_ping_bad=val;
        if (g_cslSettings->m_ping_bad<g_cslSettings->m_ping_good)
            g_cslSettings->m_ping_bad=g_cslSettings->m_ping_good;
    }
    if (config.Read(wxT("LastOutputPath"),&s)) g_cslSettings->m_outputPath=s;

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    if (config.Read(wxT("AutoFit"),&val)) g_cslSettings->m_autoFitColumns=val>0;
    if (config.Read(wxT("AutoSort"),&val)) g_cslSettings->m_autoSortColumns=val>0;
    if (version)
    {
        if (config.Read(wxT("ColMult1"),&dval)) g_cslSettings->m_colServerS1=(float)dval;
        if (config.Read(wxT("ColMult2"),&dval)) g_cslSettings->m_colServerS2=(float)dval;
        if (config.Read(wxT("ColMult3"),&dval)) g_cslSettings->m_colServerS3=(float)dval;
        if (config.Read(wxT("ColMult4"),&dval)) g_cslSettings->m_colServerS4=(float)dval;
        if (config.Read(wxT("ColMult5"),&dval)) g_cslSettings->m_colServerS5=(float)dval;
        if (config.Read(wxT("ColMult6"),&dval)) g_cslSettings->m_colServerS6=(float)dval;
        if (config.Read(wxT("ColMult7"),&dval)) g_cslSettings->m_colServerS7=(float)dval;
        if (config.Read(wxT("ColMult8"),&dval)) g_cslSettings->m_colServerS8=(float)dval;
        if (config.Read(wxT("ColMult9"),&dval)) g_cslSettings->m_colServerS9=(float)dval;
    }
    if (config.Read(wxT("ColourEmpty"),&val)) g_cslSettings->m_colServerEmpty=INT2COLOUR(val);
    if (config.Read(wxT("ColourOffline"),&val)) g_cslSettings->m_colServerOff=INT2COLOUR(val);
    if (config.Read(wxT("ColourFull"),&val)) g_cslSettings->m_colServerFull=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM1"),&val)) g_cslSettings->m_colServerMM1=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM2"),&val)) g_cslSettings->m_colServerMM2=INT2COLOUR(val);
    if (config.Read(wxT("ColourMM3"),&val)) g_cslSettings->m_colServerMM3=INT2COLOUR(val);
    if (config.Read(wxT("ColourSearch"),&val)) g_cslSettings->m_colServerHigh=INT2COLOUR(val);
    if (config.Read(wxT("ColourPlaying"),&val)) g_cslSettings->m_colServerPlay=INT2COLOUR(val);
    if (config.Read(wxT("ColourStripes"),&val)) g_cslSettings->m_colInfoStripe=INT2COLOUR(val);

    /* Client */
    config.SetPath(wxT("/Clients"));
    if (config.Read(wxT("BinarySB"),&s)) g_cslSettings->m_clientBinSB=s;
    if (config.Read(wxT("OptionsSB"),&s)) g_cslSettings->m_clientOptsSB=s;
    if (config.Read(wxT("PathSB"),&s)) g_cslSettings->m_gamePathSB=s;
    if (config.Read(wxT("PrivateConfigSB"),&val)) g_cslSettings->m_privConfSB=val>0;
    if (config.Read(wxT("BinaryAC"),&s)) g_cslSettings->m_clientBinAC=s;
    if (config.Read(wxT("OptionsAC"),&s)) g_cslSettings->m_clientOptsAC=s;
    if (config.Read(wxT("PathAC"),&s)) g_cslSettings->m_gamePathAC=s;
    if (config.Read(wxT("BinaryBF"),&s)) g_cslSettings->m_clientBinBF=s;
    if (config.Read(wxT("OptionsBF"),&s)) g_cslSettings->m_clientOptsBF=s;
    if (config.Read(wxT("PrivateConfigBF"),&val)) g_cslSettings->m_privConfBF=val>0;
    if (config.Read(wxT("PathBF"),&s)) g_cslSettings->m_gamePathBF=s;
    if (config.Read(wxT("BinaryCB"),&s)) g_cslSettings->m_clientBinCB=s;
    if (config.Read(wxT("OptionsCB"),&s)) g_cslSettings->m_clientOptsCB=s;
    if (config.Read(wxT("PathCB"),&s)) g_cslSettings->m_gamePathCB=s;
    if (config.Read(wxT("MinPlaytime"),&val))
    {
        if (val<CSL_MIN_PLAYTIME_MIN || val>CSL_MIN_PLAYTIME_MAX)
            val=CSL_MIN_PLAYTIME_STD;
        g_cslSettings->m_minPlaytime=val;
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
    config.Write(wxT("UpdateInterval"),(long int)g_cslSettings->m_updateInterval);
    config.Write(wxT("NoUpdatePlaying"),g_cslSettings->m_dontUpdatePlaying);
    config.Write(wxT("ShowFilter"),g_cslSettings->m_showFilter);
    config.Write(wxT("FilterMaster"),(long int)g_cslSettings->m_filterMaster);
    config.Write(wxT("FilterFavourites"),(long int)g_cslSettings->m_filterFavourites);
    config.Write(wxT("WaitOnFullServer"),(long int)g_cslSettings->m_waitServerFull);
    config.Write(wxT("PingGood"),(long int)g_cslSettings->m_ping_good);
    config.Write(wxT("PingBad"),(long int)g_cslSettings->m_ping_bad);
    config.Write(wxT("LastOutputPath"),g_cslSettings->m_outputPath);

    /* ListCtrl */
    config.SetPath(wxT("/List"));
    config.Write(wxT("AutoFit"),g_cslSettings->m_autoFitColumns);
    config.Write(wxT("AutoSort"),g_cslSettings->m_autoSortColumns);
    config.Write(wxT("ColMult1"),g_cslSettings->m_colServerS1);
    config.Write(wxT("ColMult2"),g_cslSettings->m_colServerS2);
    config.Write(wxT("ColMult3"),g_cslSettings->m_colServerS3);
    config.Write(wxT("ColMult4"),g_cslSettings->m_colServerS4);
    config.Write(wxT("ColMult5"),g_cslSettings->m_colServerS5);
    config.Write(wxT("ColMult6"),g_cslSettings->m_colServerS6);
    config.Write(wxT("ColMult7"),g_cslSettings->m_colServerS7);
    config.Write(wxT("ColMult8"),g_cslSettings->m_colServerS8);
    config.Write(wxT("ColMult9"),g_cslSettings->m_colServerS9);
    config.Write(wxT("ColourEmpty"),COLOUR2INT(g_cslSettings->m_colServerEmpty));
    config.Write(wxT("ColourOffline"),COLOUR2INT(g_cslSettings->m_colServerOff));
    config.Write(wxT("ColourFull"),COLOUR2INT(g_cslSettings->m_colServerFull));
    config.Write(wxT("ColourMM1"),COLOUR2INT(g_cslSettings->m_colServerMM1));
    config.Write(wxT("ColourMM2"),COLOUR2INT(g_cslSettings->m_colServerMM2));
    config.Write(wxT("ColourMM3"),COLOUR2INT(g_cslSettings->m_colServerMM3));
    config.Write(wxT("ColourSearch"),COLOUR2INT(g_cslSettings->m_colServerHigh));
    config.Write(wxT("ColourPlaying"),COLOUR2INT(g_cslSettings->m_colServerPlay));
    config.Write(wxT("ColourStripes"),COLOUR2INT(g_cslSettings->m_colInfoStripe));

    /* Client */
    config.SetPath(wxT("/Clients"));
    config.Write(wxT("BinarySB"),g_cslSettings->m_clientBinSB);
    config.Write(wxT("OptionsSB"),g_cslSettings->m_clientOptsSB);
    config.Write(wxT("PathSB"),g_cslSettings->m_gamePathSB);
    config.Write(wxT("PrivateConfigSB"),g_cslSettings->m_privConfSB);
    config.Write(wxT("BinaryAC"),g_cslSettings->m_clientBinAC);
    config.Write(wxT("OptionsAC"),g_cslSettings->m_clientOptsAC);
    config.Write(wxT("PathAC"),g_cslSettings->m_gamePathAC);
    config.Write(wxT("BinaryBF"),g_cslSettings->m_clientBinBF);
    config.Write(wxT("OptionsBF"),g_cslSettings->m_clientOptsBF);
    config.Write(wxT("PrivateConfigBF"),g_cslSettings->m_privConfBF);
    config.Write(wxT("PathBF"),g_cslSettings->m_gamePathBF);
    config.Write(wxT("BinaryCB"),g_cslSettings->m_clientBinCB);
    config.Write(wxT("OptionsCB"),g_cslSettings->m_clientOptsCB);
    config.Write(wxT("PathCB"),g_cslSettings->m_gamePathCB);
    config.Write(wxT("MinPlaytime"),(long int)g_cslSettings->m_minPlaytime);
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

    wxString addr,path,password,description;
    wxUint16 port=0;
    wxUint32 view=0;
    wxUint32 lastSeen=0;
    wxUint32 playLast=0;
    wxUint32 playTimeLastGame=0;
    wxUint32 playTimeTotal=0;
    wxUint32 connectedTimes=0;
    wxUint32 gt;

    wxFileConfig config(wxEmptyString,wxEmptyString,file,wxEmptyString,wxCONFIG_USE_LOCAL_FILE);

    for (gt=CSL_GAME_START+1;gt<CSL_GAME_END;gt++)
    {
        wxString s=wxT("/")+wxString(GetGameStr(gt));
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
                    config.Read(wxT("Type"),&val,CSL_MASTER_CONN_HTTP);
                    if (val==CSL_MASTER_CONN_HTTP)
                    {
                        port=port ? port:CSL_DEFAULT_MASTER_WEB_PORT;
                        if (config.Read(wxT("Path"),&path))
                            master=new CslMaster((CSL_GAMETYPE)gt,CslMasterConnection(addr,path,port));
                    }
                    else if (port)
                        master=new CslMaster((CSL_GAMETYPE)gt,CslMasterConnection(addr,port));
                }
            }
            if (!master)
                break;

            if (!m_engine->AddMaster(master))
            {
                delete master;
                continue;
            }
            mappings.add(new CslIDMapping(id,master->GetID()));
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
                config.Read(wxT("Port"),&val,GetDefaultPort((CSL_GAMETYPE)gt)); port=val;
                config.Read(wxT("Password"),&password);
                config.Read(wxT("Description"),&description);

                while (config.Read(wxString::Format(wxT("Master%d"),id++),&val))
                    ids.add(val);
                if (ids.length()==0)
                    ids.add(-1);

                if (config.Read(wxT("View"),&val))
                {
                    view=val;
                    if (config.Read(wxT("LastSeen"),&val))
                        lastSeen=(wxUint32)val;
                    if (config.Read(wxT("PlayLast"),&val))
                        playLast=(wxUint32)val;
                    if (config.Read(wxT("PlayTimeLastGame"),&val))
                        playTimeLastGame=(wxUint32)val;
                    if (config.Read(wxT("PlayTimeTotal"),&val))
                        playTimeTotal=(wxUint32)val;
                    if (config.Read(wxT("ConnectedTimes"),&val))
                        connectedTimes=(wxUint32)val;
                    read_server=true;
                }
            }

            if (!read_server)
                break;

            loopv(ids)
            {
                CslServerInfo *info=new CslServerInfo((CSL_GAMETYPE)gt,addr,port,view,
                                                      lastSeen,playLast,playTimeLastGame,
                                                      playTimeTotal,connectedTimes,
                                                      description,password);

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

                CslServerInfo *ret=m_engine->AddServer(info,id);
                if (!ret || ret!=info)
                    delete info;
                else
                    tsc++;
            }
        }
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
        {
            /*
            wxMessageBox(wxString::Format(_("Cannot create directory: \"%s\"" \
                                            "for saving server information."),
                                          datadir.c_str()),CSL_ERROR_STR,wxICON_ERROR,this);
            */
            return;
        }

    wxFileConfig config(wxEmptyString,wxEmptyString,s+PATHDIV+wxT("servers.ini"),
                        wxEmptyString,wxCONFIG_USE_LOCAL_FILE);
    config.SetUmask(0077);
    config.DeleteAll();

    wxInt32 mc,sc;
    CslMaster *master;
    CslServerInfo *info;
    CslGame *game;
    vector<CslMaster*> *masters;
    vector<CslServerInfo*> *servers;
    vector<CslGame*> *games=m_engine->GetGames();

    if (!games->length())
        return;

    config.SetPath(wxT("/Version"));
    config.Write(wxT("Version"),CSL_SERVERCONFIG_VERSION);

    servers=m_engine->GetFavourites();

    loopv(*games)
    {
        game=games->at(i);
        s=wxT("/")+game->GetName();
        config.SetPath(s);

        masters=game->GetMasters();

        mc=0;
        loopvj(*masters)
        {
            master=masters->at(j);
            CslMasterConnection& connection=master->GetConnection();

            config.SetPath(s+s.Format(wxT("/Master/%d"),mc++));
            config.Write(wxT("Type"),connection.GetType());
            config.Write(wxT("Address"),connection.GetAddress());
            config.Write(wxT("Port"),connection.GetPort());
            config.Write(wxT("Path"),connection.GetPath());
            config.Write(wxT("ID"),master->GetID());
        }

        servers=game->GetServers();
        if (servers->length()==0)
            continue;

        sc=0;
        loopvk(*servers)
        {
            info=servers->at(k);
            config.SetPath(s+s.Format(wxT("/Server/%d"),sc++));
            config.Write(wxT("Address"),info->m_host);
            config.Write(wxT("Port"),info->m_port);
            config.Write(wxT("Password"),info->m_password);
            config.Write(wxT("Description"),info->m_descOld);

            loopvj(info->m_masterIDs)
            {
                config.Write(wxString::Format(wxT("Master%d"),j),info->m_masterIDs[j]);
            }
            config.Write(wxT("View"),(int)info->m_view);
            config.Write(wxT("LastSeen"),(int)info->m_lastSeen);
            config.Write(wxT("PlayLast"),(int)info->m_playLast);
            config.Write(wxT("PlayTimeLastGame"),(int)info->m_playTimeLastGame);
            config.Write(wxT("PlayTimeTotal"),(int)info->m_playTimeTotal);
            config.Write(wxT("ConnectedTimes"),(int)info->m_connectedTimes);
        }
    }
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
    //TODO wxApp::SetExitOnFrameDelete(false);
#endif

    wxString name=wxString::Format(wxT(".%s-%s.lock"),CSL_NAME_SHORT_STR,wxGetUserId().c_str());
    m_single=new wxSingleInstanceChecker(name);

    if (m_single->IsAnotherRunning())
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
