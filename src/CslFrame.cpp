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

    CSL_CHECK_FILTER_FULL,
    CSL_CHECK_FILTER_EMPTY,
    CSL_CHECK_FILTER_NONEMPTY,
    CSL_CHECK_FILTER_OFFLINE,
    CSL_CHECK_FILTER_MM2,
    CSL_CHECK_FILTER_MM3,
    CSL_CHECK_FILTER_VERSION,
    CSL_CHOICE_FILTER_VERSION,
    CSL_CHECK_FILTER_FAVOURITES,
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
    CSL_EVT_PONG(wxID_ANY,CslFrame::OnPong)
    EVT_TIMER(wxID_ANY,CslFrame::OnTimer)
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

    LoadSettings();
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

    // begin wxGlade: CslFrame::CslFrame
    panel_frame = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);
    panel_main = new wxPanel(panel_frame, wxID_ANY);
    splitter_main = new wxSplitterWindow(panel_main, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_BORDER);
    pane_main_right = new wxPanel(splitter_main, wxID_ANY);
    splitter_lists = new wxSplitterWindow(pane_main_right, CSL_SPLITTER_LISTS, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
    pane_favourites = new wxPanel(splitter_lists, wxID_ANY);
    pane_master = new wxPanel(splitter_lists, wxID_ANY);
    panel_search = new wxPanel(pane_master, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);
    pane_main_left = new wxPanel(splitter_main, wxID_ANY);
    splitter_games_info = new wxSplitterWindow(pane_main_left, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
    pane_info = new wxPanel(splitter_games_info, wxID_ANY);
    sizer_filter_staticbox = new wxStaticBox(pane_main_left, -1, _("Filter out these servers"));
    pane_games = new wxPanel(splitter_games_info, wxID_ANY);
    tree_ctrl_games = new wxTreeCtrl(pane_games, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_NO_LINES|wxTR_LINES_AT_ROOT|wxTR_DEFAULT_STYLE);
    list_ctrl_info = new CslListCtrlInfo(pane_info, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER);
    checkbox_filter_full = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_FULL, _("F&ull"));
    checkbox_filter_offline = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_OFFLINE, _("&Offline"));
    checkbox_filter_empty = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_EMPTY, _("&Empty"));
    checkbox_filter_mm2 = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_MM2, _("Mastermode &2"));
    checkbox_filter_nonempty = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_NONEMPTY, _("&Not empty"));
    checkbox_filter_mm3 = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_MM3, _("Mastermode &3"));
    checkbox_filter_favourites = new wxCheckBox(pane_main_left, CSL_CHECK_FILTER_FAVOURITES, _("Filter fa&vourites as well"));
    button_filter_reset = new wxButton(pane_main_left, CSL_BUTTON_FILTER_RESET, _("&Reset"));
    list_ctrl_master = new CslListCtrlServer(pane_master, CSL_LIST_MASTER, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    text_ctrl_search = new wxTextCtrl(panel_search, CSL_TEXT_SEARCH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_RICH2);
    list_ctrl_favourites = new CslListCtrlServer(pane_favourites, CSL_LIST_FAVOURITE, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

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
            master=new CslMaster(CSL_GAME_SB,CSL_DEFAULT_MASTER_SB,CSL_DEFAULT_MASTER_PATH_SB,true);
            game=m_engine->AddMaster(master);
            m_engine->SetCurrentGame(game,NULL);
            TreeAddGame(game,true);

            master=new CslMaster(CSL_GAME_AC,CSL_DEFAULT_MASTER_AC,CSL_DEFAULT_MASTER_PATH_AC,true);
            game=m_engine->AddMaster(master);
            TreeAddGame(game);

            master=new CslMaster(CSL_GAME_BF,CSL_DEFAULT_MASTER_BF,CSL_DEFAULT_MASTER_PATH_BF,true);
            game=m_engine->AddMaster(master);
            TreeAddGame(game);

            master=new CslMaster(CSL_GAME_CB,CSL_DEFAULT_MASTER_CB,CSL_DEFAULT_MASTER_PATH_CB,true);
            game=m_engine->AddMaster(master);
            TreeAddGame(game);

            CslServerInfo *info;
            info=new CslServerInfo();
            info->CreateFavourite(CSL_DEFAULT_SERVER_ADDR_SB1,CSL_GAME_SB);
            m_engine->AddServer(info,-1);
            info=new CslServerInfo();
            info->CreateFavourite(CSL_DEFAULT_SERVER_ADDR_SB2,CSL_GAME_SB);
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
                       "Please restart the application."),
                     _("Fatal error!"),wxICON_ERROR,this);
        delete m_engine;
        m_engine=NULL;
    }

    m_extendedDlg->ListInit(m_engine);

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
}

void CslFrame::set_properties()
{
    // begin wxGlade: CslFrame::set_properties
    SetTitle(_("Cube Server Lister"));
    // end wxGlade

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

    // wxMAC: have to set minsize of the listctrl to prevent
    //        hiding of the search panel while dragging splitter
    list_ctrl_master->SetMinSize(wxSize(0,0));

    SetMinSize(wxSize(640,480));

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
    CslStatusBar *statusBar=new CslStatusBar(this);
    SetStatusBar(statusBar);
    CslStatusBar::InitBar(statusBar);

    list_ctrl_master->ListInit(m_engine,list_ctrl_info,NULL,list_ctrl_favourites,
                               m_extendedDlg,g_cslSettings->m_filterFlags);
    list_ctrl_favourites->ListInit(m_engine,list_ctrl_info,list_ctrl_master,NULL,m_extendedDlg,
                                   g_cslSettings->m_filterFavourites ? g_cslSettings->m_filterFlags : 0);

    // begin wxGlade: CslFrame::do_layout
    wxBoxSizer* sizer_frame = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer_panel_frame = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer_panel = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer_main_right = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer_list_favourites = new wxBoxSizer(wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_master = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_search = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_main_left = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_filter = new wxStaticBoxSizer(sizer_filter_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_filter = new wxFlexGridSizer(4, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_info = new wxFlexGridSizer(1, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_games_filter = new wxFlexGridSizer(1, 1, 0, 0);
    grid_sizer_games_filter->Add(tree_ctrl_games, 1, wxEXPAND, 4);
    pane_games->SetSizer(grid_sizer_games_filter);
    grid_sizer_games_filter->AddGrowableRow(0);
    grid_sizer_games_filter->AddGrowableCol(0);
    grid_sizer_info->Add(list_ctrl_info, 1, wxEXPAND, 4);
    pane_info->SetSizer(grid_sizer_info);
    grid_sizer_info->AddGrowableRow(0);
    grid_sizer_info->AddGrowableCol(0);
    splitter_games_info->SplitHorizontally(pane_games, pane_info);
    grid_sizer_main_left->Add(splitter_games_info, 1, wxEXPAND, 4);
    grid_sizer_filter->Add(checkbox_filter_full, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(1, 1, 0, wxEXPAND, 0);
    grid_sizer_filter->Add(checkbox_filter_offline, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(checkbox_filter_empty, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(1, 1, 0, wxEXPAND, 0);
    grid_sizer_filter->Add(checkbox_filter_mm2, 0, wxLEFT|wxRIGHT|wxTOP|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(checkbox_filter_nonempty, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(1, 1, 0, wxEXPAND, 0);
    grid_sizer_filter->Add(checkbox_filter_mm3, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(checkbox_filter_favourites, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_filter->Add(1, 1, 0, wxEXPAND, 0);
    grid_sizer_filter->Add(button_filter_reset, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_filter->AddGrowableCol(1);
    sizer_filter->Add(grid_sizer_filter, 1, wxEXPAND, 0);
    grid_sizer_main_left->Add(sizer_filter, 1, wxTOP|wxEXPAND, 4);
    pane_main_left->SetSizer(grid_sizer_main_left);
    grid_sizer_main_left->AddGrowableRow(0);
    grid_sizer_main_left->AddGrowableCol(0);
    grid_sizer_master->Add(list_ctrl_master, 1, wxEXPAND, 5);
    wxStaticText* label_search_static = new wxStaticText(panel_search, wxID_ANY, _("Search:"));
    grid_sizer_search->Add(label_search_static, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_search->Add(text_ctrl_search, 0, wxLEFT|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    panel_search->SetSizer(grid_sizer_search);
    grid_sizer_search->AddGrowableCol(2);
    grid_sizer_master->Add(panel_search, 1, wxALL|wxEXPAND, 4);
    pane_master->SetSizer(grid_sizer_master);
    grid_sizer_master->AddGrowableRow(0);
    grid_sizer_master->AddGrowableCol(0);
    sizer_list_favourites->Add(list_ctrl_favourites, 1, wxEXPAND, 3);
    pane_favourites->SetSizer(sizer_list_favourites);
    splitter_lists->SplitHorizontally(pane_master, pane_favourites);
    sizer_main_right->Add(splitter_lists, 1, wxEXPAND, 4);
    pane_main_right->SetSizer(sizer_main_right);
    splitter_main->SplitVertically(pane_main_left, pane_main_right);
    sizer_panel->Add(splitter_main, 1, wxEXPAND, 0);
    panel_main->SetSizer(sizer_panel);
    sizer_panel_frame->Add(panel_main, 1, wxALL|wxEXPAND, 4);
    panel_frame->SetSizer(sizer_panel_frame);
    sizer_frame->Add(panel_frame, 1, wxEXPAND, 0);
    SetSizer(sizer_frame);
    sizer_frame->Fit(this);
    Layout();
    // end wxGlade

#ifdef __WXMAC__
    wxSizerItem *sitem=grid_sizer_main_left->GetItem(sizer_filter);
    sitem->SetFlag(wxLEFT|wxTOP|wxBOTTOM|wxEXPAND);
    sitem->SetBorder(4);
#endif

    wxInt32 bboffset=0;
#ifdef __WXMSW__
    bboffset=2;
#endif
    grid_sizer_search->Insert(0,bitmap_button_search_clear,0,
                              wxRIGHT|wxLEFT|wxTOP|wxALIGN_CENTER_VERTICAL,
                              bboffset);

    m_sizerMaster=grid_sizer_master;
    m_sizerLeft=grid_sizer_main_left;
    m_sizerSearch=grid_sizer_search;
    m_sizerFilter=sizer_filter;

    SetSize(g_cslSettings->m_frameSize);
    CentreOnScreen();

    ToggleSearchBar();
    ToggleFilter();
    ToggleSplitterUpdate();

// only set splitter gravity on wxMAC here - it doesn't
// work under wxGTK and EVT_SHOW is not send on wxMAC
#ifdef __WXMAC__
    splitter_games_info->SetSashGravity(1.0f);
    splitter_lists->SetSashGravity(0.8f);
#endif
    splitter_main->SetMinimumPaneSize(sizer_filter->GetMinSize().GetWidth()+4);
    splitter_games_info->SetMinimumPaneSize(20);
    splitter_lists->SetMinimumPaneSize(100);
    splitter_main->SetSashPosition(g_cslSettings->m_splitterMainPos);
    splitter_games_info->SetSashPosition(g_cslSettings->m_splitterGamePos);
    splitter_lists->SetSashPosition(g_cslSettings->m_splitterListPos);
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
    CslMenu::AddItemToMenu(menu,MENU_VIEW_SEARCH,_("Show &search bar\tCTRL+S"),
                           wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItemToMenu(menu,MENU_VIEW_FILTER,_("Show &filter\tCTRL+F"),
                           wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItemToMenu(menu,MENU_VIEW_OUTPUT,_("Show &game output\tCTRL+O"),
                           wxART_NONE,wxITEM_CHECK);
    menu->AppendSeparator();
    CslMenu::AddItemToMenu(menu,MENU_VIEW_AUTO_SORT,_("Sort &lists automatically\tCTRL+L"),
                           wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItemToMenu(menu,MENU_VIEW_AUTO_FIT,_("Fit columns on window &resize"),
                           wxART_NONE,wxITEM_CHECK);
    CslMenu::AddItemToMenu(menu,MENU_VIEW_SPLITTER_LIVE,_("Redraw while dragging spli&tters"),
                           wxART_NONE,wxITEM_CHECK);
    m_menubar->Append(menu,_("&View"));

    menu=new wxMenu();
    CslMenu::AddItemToMenu(menu,wxID_ABOUT,_("A&bout"),wxART_ABOUT);
    m_menubar->Append(menu,_("&Help"));

    SetMenuBar(m_menubar);
    m_menu=new CslMenu(m_menubar);
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
    wxUint32 u=0;
    bool checked=false;

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

            CslMaster *master=new CslMaster;
            master->Create(type);

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

        case MENU_VIEW_SEARCH:
            g_cslSettings->m_showSearch=event.IsChecked();
            ToggleSearchBar();
            break;

        case MENU_VIEW_FILTER:
            g_cslSettings->m_showFilter=event.IsChecked();
            ToggleFilter();
            break;

        case MENU_VIEW_OUTPUT:
            m_outputDlg->Show(!m_outputDlg->IsShown());
            break;

        case MENU_VIEW_AUTO_SORT:
            g_cslSettings->m_autoSortColumns=event.IsChecked();
            list_ctrl_master->ToggleSortArrow();
            list_ctrl_favourites->ToggleSortArrow();
            break;

        case MENU_VIEW_AUTO_FIT:
            g_cslSettings->m_autoFitColumns=event.IsChecked();
            break;

        case MENU_VIEW_SPLITTER_LIVE:
            g_cslSettings->m_splitterLive=event.IsChecked();
            ToggleSplitterUpdate();
            break;

        case wxID_ABOUT:
        {
            CslDlgAbout *dlg=new CslDlgAbout(this);
            dlg->Show();
            break;
        }

        case CSL_BUTTON_SEARCH_CLOSE:
            g_cslSettings->m_showSearch=false;
            CslMenu::CheckMenuItem(MENU_VIEW_SEARCH,false);
            ToggleSearchBar();
            break;

        case CSL_TEXT_SEARCH:
        {
            wxString s=event.GetString();

            if (list_ctrl_master->ListSearch(s)||s.IsEmpty())
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

        case CSL_CHECK_FILTER_OFFLINE:
            u=CSL_FILTER_OFFLINE;
            checked=event.IsChecked();
        case CSL_CHECK_FILTER_FULL:
            if (!u)
            {
                u=CSL_FILTER_FULL;
                checked=event.IsChecked();
            }
        case CSL_CHECK_FILTER_EMPTY:
            if (!u)
            {
                u=CSL_FILTER_EMPTY;
                checked=event.IsChecked();
            }
        case CSL_CHECK_FILTER_NONEMPTY:
            if (!u)
            {
                u=CSL_FILTER_NONEMPTY;
                checked=event.IsChecked();
            }
        case CSL_CHECK_FILTER_MM2:
            if (!u)
            {
                u=CSL_FILTER_MM2;
                checked=event.IsChecked();
            }
        case CSL_CHECK_FILTER_MM3:
            if (!u)
            {
                u=CSL_FILTER_MM3;
                checked=event.IsChecked();
            }
            if (checked)
                g_cslSettings->m_filterFlags|=u;
            else
                g_cslSettings->m_filterFlags&=~u;

            list_ctrl_master->ListFilter(g_cslSettings->m_filterFlags);
            if (g_cslSettings->m_filterFavourites && checkbox_filter_favourites->IsChecked())
                list_ctrl_favourites->ListFilter(g_cslSettings->m_filterFlags);
            break;

        case CSL_CHECK_FILTER_FAVOURITES:
            u=0;
            g_cslSettings->m_filterFavourites=event.IsChecked();
            if (g_cslSettings->m_filterFavourites)
                u=g_cslSettings->m_filterFlags;
            list_ctrl_favourites->ListFilter(u);
            break;

        case CSL_BUTTON_FILTER_RESET:
            g_cslSettings->m_filterFlags=0;
            UpdateFilterCheckBoxes();
            list_ctrl_master->ListFilter(0);
            if (checkbox_filter_favourites->IsChecked())
                list_ctrl_favourites->ListFilter(0);
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
            CslConnectionState::Reset();
        else if (g_cslSettings->m_showSearch && FindFocus()==text_ctrl_search)
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
    splitter_games_info->SetSashGravity(1.0f);
    splitter_lists->SetSashGravity(0.8f);
    list_ctrl_master->ListAdjustSize(list_ctrl_master->GetClientSize(),true);
    list_ctrl_favourites->ListAdjustSize(list_ctrl_favourites->GetClientSize(),true);
}

void CslFrame::OnClose(wxCloseEvent& event)
{
    if (event.GetEventObject()==m_outputDlg)
    {
        CslMenu::CheckMenuItem(MENU_VIEW_OUTPUT,false);
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

void CslFrame::ToggleSearchBar()
{
    if (g_cslSettings->m_showSearch && !panel_search->IsShown())
    {
        m_sizerMaster->Insert(1,panel_search,1,wxEXPAND|wxALL,4);
        panel_search->Show();
        text_ctrl_search->SetFocus();
    }
    else if (!g_cslSettings->m_showSearch && panel_search->IsShown())
    {
        panel_search->Hide();
        text_ctrl_search->Clear();
        text_ctrl_search->SetBackgroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOW));
        list_ctrl_master->ListSearch(wxEmptyString);
        m_sizerMaster->Detach(panel_search);
    }
    else
        return;

    m_sizerMaster->Layout();
}

void CslFrame::ToggleFilter()
{
    if (g_cslSettings->m_showFilter && !m_sizerLeft->IsShown(m_sizerFilter))
    {
        //m_sizerLeft->Detach(m_sizerFilter);
        m_sizerLeft->Show(m_sizerFilter);
    }
    else if (!g_cslSettings->m_showFilter && m_sizerLeft->IsShown(m_sizerFilter))
    {
        //m_sizerLeft->Insert(1,m_sizerFilter,1,wxEXPAND|wxALL,4);
        m_sizerLeft->Hide(m_sizerFilter,true);
    }
    m_sizerLeft->Layout();
}

void CslFrame::ToggleSplitterUpdate()
{
    wxUint32 styleMain=splitter_main->GetWindowStyle();
    wxUint32 styleGame=splitter_games_info->GetWindowStyle();
    wxUint32 styleList=splitter_lists->GetWindowStyle();

    if (g_cslSettings->m_splitterLive)
    {
        styleMain|=wxSP_LIVE_UPDATE;
        styleGame|=wxSP_LIVE_UPDATE;
        styleList|=wxSP_LIVE_UPDATE;
    }
    else
    {
        styleMain&=~wxSP_LIVE_UPDATE;
        styleGame&=~wxSP_LIVE_UPDATE;
        styleList&=~wxSP_LIVE_UPDATE;
    }

    splitter_main->SetWindowStyle(styleMain);
    splitter_games_info->SetWindowStyle(styleGame);
    splitter_lists->SetWindowStyle(styleList);
}

void CslFrame::UpdateFilterCheckBoxes()
{
    checkbox_filter_offline->SetValue((g_cslSettings->m_filterFlags&CSL_FILTER_OFFLINE)>0);
    checkbox_filter_full->SetValue((g_cslSettings->m_filterFlags&CSL_FILTER_FULL)>0);
    checkbox_filter_empty->SetValue((g_cslSettings->m_filterFlags&CSL_FILTER_EMPTY)>0);
    checkbox_filter_nonempty->SetValue((g_cslSettings->m_filterFlags&CSL_FILTER_NONEMPTY)>0);
    checkbox_filter_mm2->SetValue((g_cslSettings->m_filterFlags&CSL_FILTER_MM2)>0);
    checkbox_filter_mm3->SetValue((g_cslSettings->m_filterFlags&CSL_FILTER_MM3)>0);

    checkbox_filter_favourites->SetValue(g_cslSettings->m_filterFavourites);
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

    SetStatusText(_("Sending request to master: ")+master->GetAddress(),1);
    m_timer.Stop();

    CslMenu::EnableMenuItem(MENU_MASTER_UPDATE,false);
    tree_ctrl_games->Enable(false);

    wxInt32 num=m_engine->UpdateMaster();
    if (num<0)
        SetStatusText(_("Error on update from master: ")+master->GetAddress(),1);
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
                if (data->GetGame()==game)
                    break;
                else if (master)
                    if (data->GetMaster()==master)
                        break;
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
    item=tree_ctrl_games->AppendItem(parent,master->GetAddress(),IMG_LIST_TREE_MASTER,-1,data);

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
    long int val;
    double dval;
    wxUint32 version=0;
    wxString s=wxStandardPaths().GetUserDataDir();
    wxString file=s+PATHDIV+wxT("settings.ini");

    g_cslSettings=new CslSettings;

    if (!::wxFileExists(file))
        return;

    wxFileConfig *config=new wxFileConfig(wxEmptyString,wxEmptyString,file,
                                          wxEmptyString,wxCONFIG_USE_LOCAL_FILE);

    config->SetPath(wxT("/Version"));
    if (config->Read(wxT("Version"),&val)) version=val;

    config->SetPath(wxT("/Gui"));
    if (config->Read(wxT("SizeX"),&val)) g_cslSettings->m_frameSize.SetWidth(val);
    if (config->Read(wxT("SizeY"),&val)) g_cslSettings->m_frameSize.SetHeight(val);
    if (config->Read(wxT("SplitterMain"),&val)) g_cslSettings->m_splitterMainPos=val;
    if (config->Read(wxT("SplitterGame"),&val)) g_cslSettings->m_splitterGamePos=val;
    if (config->Read(wxT("SplitterList"),&val)) g_cslSettings->m_splitterListPos=val;
    if (config->Read(wxT("SplitterLiveUpdate"),&val)) g_cslSettings->m_splitterLive=val>0;
    if (config->Read(wxT("UpdateInterval"),&val))
    {
        if (val<CSL_UPDATE_INTERVAL_MIN || val>CSL_UPDATE_INTERVAL_MAX)
            val=CSL_UPDATE_INTERVAL_MIN;
        g_cslSettings->m_updateInterval=val;
    }
    if (config->Read(wxT("NoUpdatePlaying"),&val)) g_cslSettings->m_dontUpdatePlaying=val>0;
    if (config->Read(wxT("ShowSearch"),&val)) g_cslSettings->m_showSearch=val>0;
    if (config->Read(wxT("ShowFilter"),&val)) g_cslSettings->m_showFilter=val>0;
    if (config->Read(wxT("FilterFavourites"),&val)) g_cslSettings->m_filterFavourites=val>0;
    if (config->Read(wxT("Filter"),&val)) g_cslSettings->m_filterFlags=val;
    if (config->Read(wxT("WaitOnFullServer"),&val))
    {
        if (val<CSL_WAIT_SERVER_FULL_MIN || val>CSL_WAIT_SERVER_FULL_MAX)
            val=CSL_WAIT_SERVER_FULL_STD;
        g_cslSettings->m_waitServerFull=val;
    }
    if (config->Read(wxT("PingGood"),&val))
    {
        g_cslSettings->m_ping_good=val;
        if (g_cslSettings->m_ping_good>9999)
            g_cslSettings->m_ping_good=CSL_PING_GOOD_STD;
    }
    if (config->Read(wxT("PingBad"),&val))
    {
        g_cslSettings->m_ping_bad=val;
        if (g_cslSettings->m_ping_bad<g_cslSettings->m_ping_good)
            g_cslSettings->m_ping_bad=g_cslSettings->m_ping_good;
    }
    if (config->Read(wxT("LastOutputPath"),&s)) g_cslSettings->m_outputPath=s;

    /* ListCtrl */
    config->SetPath(wxT("/List"));
    if (config->Read(wxT("AutoFit"),&val)) g_cslSettings->m_autoFitColumns=val>0;
    if (config->Read(wxT("AutoSort"),&val)) g_cslSettings->m_autoSortColumns=val>0;
    if (version)
    {
        if (config->Read(wxT("ColMult1"),&dval)) g_cslSettings->m_colServerS1=(float)dval;
        if (config->Read(wxT("ColMult2"),&dval)) g_cslSettings->m_colServerS2=(float)dval;
        if (config->Read(wxT("ColMult3"),&dval)) g_cslSettings->m_colServerS3=(float)dval;
        if (config->Read(wxT("ColMult4"),&dval)) g_cslSettings->m_colServerS4=(float)dval;
        if (config->Read(wxT("ColMult5"),&dval)) g_cslSettings->m_colServerS5=(float)dval;
        if (config->Read(wxT("ColMult6"),&dval)) g_cslSettings->m_colServerS6=(float)dval;
        if (config->Read(wxT("ColMult7"),&dval)) g_cslSettings->m_colServerS7=(float)dval;
        if (config->Read(wxT("ColMult8"),&dval)) g_cslSettings->m_colServerS8=(float)dval;
        if (config->Read(wxT("ColMult9"),&dval)) g_cslSettings->m_colServerS9=(float)dval;
    }
    if (config->Read(wxT("ColourEmpty"),&val)) g_cslSettings->m_colServerEmpty=INT2COLOUR(val);
    if (config->Read(wxT("ColourOffline"),&val)) g_cslSettings->m_colServerOff=INT2COLOUR(val);
    if (config->Read(wxT("ColourFull"),&val)) g_cslSettings->m_colServerFull=INT2COLOUR(val);
    if (config->Read(wxT("ColourMM1"),&val)) g_cslSettings->m_colServerMM1=INT2COLOUR(val);
    if (config->Read(wxT("ColourMM2"),&val)) g_cslSettings->m_colServerMM2=INT2COLOUR(val);
    if (config->Read(wxT("ColourMM3"),&val)) g_cslSettings->m_colServerMM3=INT2COLOUR(val);
    if (config->Read(wxT("ColourSearch"),&val)) g_cslSettings->m_colServerHigh=INT2COLOUR(val);
    if (config->Read(wxT("ColourPlaying"),&val)) g_cslSettings->m_colServerPlay=INT2COLOUR(val);
    if (config->Read(wxT("ColourStripes"),&val)) g_cslSettings->m_colInfoStripe=INT2COLOUR(val);

    /* Client */
    config->SetPath(wxT("/Clients"));
    if (config->Read(wxT("BinarySB"),&s)) g_cslSettings->m_clientBinSB=s;
    if (config->Read(wxT("OptionsSB"),&s)) g_cslSettings->m_clientOptsSB=s;
    if (config->Read(wxT("PathSB"),&s)) g_cslSettings->m_gamePathSB=s;
    if (config->Read(wxT("PrivateConfigSB"),&val)) g_cslSettings->m_privConfSB=val>0;
    if (config->Read(wxT("BinaryAC"),&s)) g_cslSettings->m_clientBinAC=s;
    if (config->Read(wxT("OptionsAC"),&s)) g_cslSettings->m_clientOptsAC=s;
    if (config->Read(wxT("PathAC"),&s)) g_cslSettings->m_gamePathAC=s;
    if (config->Read(wxT("BinaryBF"),&s)) g_cslSettings->m_clientBinBF=s;
    if (config->Read(wxT("OptionsBF"),&s)) g_cslSettings->m_clientOptsBF=s;
    if (config->Read(wxT("PrivateConfigBF"),&val)) g_cslSettings->m_privConfBF=val>0;
    if (config->Read(wxT("PathBF"),&s)) g_cslSettings->m_gamePathBF=s;
    if (config->Read(wxT("BinaryCB"),&s)) g_cslSettings->m_clientBinCB=s;
    if (config->Read(wxT("OptionsCB"),&s)) g_cslSettings->m_clientOptsCB=s;
    if (config->Read(wxT("PathCB"),&s)) g_cslSettings->m_gamePathCB=s;
    if (config->Read(wxT("MinPlaytime"),&val))
    {
        if (val<CSL_MIN_PLAYTIME_MIN || val>CSL_MIN_PLAYTIME_MAX)
            val=CSL_MIN_PLAYTIME_STD;
        g_cslSettings->m_minPlaytime=val;
    }

    delete config;
}

void CslFrame::SaveSettings()
{
    wxString dir=wxStandardPaths().GetUserDataDir();

    if (!::wxDirExists(dir))
        ::wxMkdir(dir,0700);

    wxFileConfig *config=new wxFileConfig(wxEmptyString,wxEmptyString,
                                          dir+PATHDIV+wxT("settings.ini"),
                                          wxEmptyString,wxCONFIG_USE_LOCAL_FILE);
    config->SetUmask(0077);
    config->DeleteAll();

    wxSize size=GetSize();
    long int pos;

    config->SetPath(wxT("/Version"));
    config->Write(wxT("Version"),CSL_CONFIG_VERSION);

    /* GUI */
    config->SetPath(wxT("/Gui"));
    config->Write(wxT("SizeX"),(long int)size.GetWidth());
    config->Write(wxT("SizeY"),(long int)size.GetHeight());
    pos=splitter_main->GetSashPosition();
    config->Write(wxT("SplitterMain"),pos);
    pos=splitter_games_info->GetSashPosition();
    config->Write(wxT("SplitterGame"),pos);
    pos=splitter_lists->GetSashPosition();
    config->Write(wxT("SplitterList"),pos);
    config->Write(wxT("SplitterLiveUpdate"),(long int)g_cslSettings->m_splitterLive);
    config->Write(wxT("UpdateInterval"),(long int)g_cslSettings->m_updateInterval);
    config->Write(wxT("NoUpdatePlaying"),g_cslSettings->m_dontUpdatePlaying);
    config->Write(wxT("ShowSearch"),g_cslSettings->m_showSearch);
    config->Write(wxT("ShowFilter"),g_cslSettings->m_showFilter);
    config->Write(wxT("FilterFavourites"),g_cslSettings->m_filterFavourites);
    config->Write(wxT("Filter"),(long int)g_cslSettings->m_filterFlags);
    config->Write(wxT("WaitOnFullServer"),(long int)g_cslSettings->m_waitServerFull);
    config->Write(wxT("PingGood"),(long int)g_cslSettings->m_ping_good);
    config->Write(wxT("PingBad"),(long int)g_cslSettings->m_ping_bad);
    config->Write(wxT("LastOutputPath"),g_cslSettings->m_outputPath);

    /* ListCtrl */
    config->SetPath(wxT("/List"));
    config->Write(wxT("AutoFit"),g_cslSettings->m_autoFitColumns);
    config->Write(wxT("AutoSort"),g_cslSettings->m_autoSortColumns);
    config->Write(wxT("ColMult1"),g_cslSettings->m_colServerS1);
    config->Write(wxT("ColMult2"),g_cslSettings->m_colServerS2);
    config->Write(wxT("ColMult3"),g_cslSettings->m_colServerS3);
    config->Write(wxT("ColMult4"),g_cslSettings->m_colServerS4);
    config->Write(wxT("ColMult5"),g_cslSettings->m_colServerS5);
    config->Write(wxT("ColMult6"),g_cslSettings->m_colServerS6);
    config->Write(wxT("ColMult7"),g_cslSettings->m_colServerS7);
    config->Write(wxT("ColMult8"),g_cslSettings->m_colServerS8);
    config->Write(wxT("ColMult9"),g_cslSettings->m_colServerS9);
    config->Write(wxT("ColourEmpty"),COLOUR2INT(g_cslSettings->m_colServerEmpty));
    config->Write(wxT("ColourOffline"),COLOUR2INT(g_cslSettings->m_colServerOff));
    config->Write(wxT("ColourFull"),COLOUR2INT(g_cslSettings->m_colServerFull));
    config->Write(wxT("ColourMM1"),COLOUR2INT(g_cslSettings->m_colServerMM1));
    config->Write(wxT("ColourMM2"),COLOUR2INT(g_cslSettings->m_colServerMM2));
    config->Write(wxT("ColourMM3"),COLOUR2INT(g_cslSettings->m_colServerMM3));
    config->Write(wxT("ColourSearch"),COLOUR2INT(g_cslSettings->m_colServerHigh));
    config->Write(wxT("ColourPlaying"),COLOUR2INT(g_cslSettings->m_colServerPlay));
    config->Write(wxT("ColourStripes"),COLOUR2INT(g_cslSettings->m_colInfoStripe));

    /* Client */
    config->SetPath(wxT("/Clients"));
    config->Write(wxT("BinarySB"),g_cslSettings->m_clientBinSB);
    config->Write(wxT("OptionsSB"),g_cslSettings->m_clientOptsSB);
    config->Write(wxT("PathSB"),g_cslSettings->m_gamePathSB);
    config->Write(wxT("PrivateConfigSB"),g_cslSettings->m_privConfSB);
    config->Write(wxT("BinaryAC"),g_cslSettings->m_clientBinAC);
    config->Write(wxT("OptionsAC"),g_cslSettings->m_clientOptsAC);
    config->Write(wxT("PathAC"),g_cslSettings->m_gamePathAC);
    config->Write(wxT("BinaryBF"),g_cslSettings->m_clientBinBF);
    config->Write(wxT("OptionsBF"),g_cslSettings->m_clientOptsBF);
    config->Write(wxT("PrivateConfigBF"),g_cslSettings->m_privConfBF);
    config->Write(wxT("PathBF"),g_cslSettings->m_gamePathBF);
    config->Write(wxT("BinaryCB"),g_cslSettings->m_clientBinCB);
    config->Write(wxT("OptionsCB"),g_cslSettings->m_clientOptsCB);
    config->Write(wxT("PathCB"),g_cslSettings->m_gamePathCB);
    config->Write(wxT("MinPlaytime"),(long int)g_cslSettings->m_minPlaytime);

    delete config;
}

bool CslFrame::LoadServers(wxUint32 *numm,wxUint32 *nums)
{
    long int val;
    bool read_master,read_server;
    wxUint32 mc=0,sc=0,tmc=0,tsc=0;
    wxFileConfig *config=NULL;
    CslMaster *master=NULL;
    vector<CslIDMapping*> mappings;
    vector<wxInt32> ids;

    wxString addr,path,password,description;
    wxUint32 view=0;
    wxUint32 lastSeen=0;
    wxUint32 playLast=0;
    wxUint32 playTimeLastGame=0;
    wxUint32 playTimeTotal=0;
    wxUint32 connectedTimes=0;

    wxString s=wxStandardPaths().GetUserDataDir();
    wxString file=s+PATHDIV+wxT("servers.ini");

    if (!::wxFileExists(file))
        goto finish;

    config=new wxFileConfig(wxEmptyString,wxEmptyString,file,
                            wxEmptyString,wxCONFIG_USE_LOCAL_FILE);

    for (wxUint32 g=CSL_GAME_START+1;g<CSL_GAME_END;g++)
    {
        wxString s=wxT("/")+wxString(GetGameStr(g));
        config->SetPath(s);

        mc=0;
        read_master=true;
        while (read_master)
        {
            config->SetPath(s+wxString::Format(wxT("/Master/%d"),mc++));

            read_master=false;
            if (config->Read(wxT("address"),&addr))
            {
                if (config->Read(wxT("path"),&path))
                    if (config->Read(wxT("ID"),&val))
                        read_master=true;
            }
            if (!read_master)
                break;

            master=new CslMaster((CSL_GAMETYPE)g,addr,path);
            if (!m_engine->AddMaster(master))
            {
                delete master;
                continue;
            }
            mappings.add(new CslIDMapping(val,master->GetID()));
            tmc++;
        }

        sc=0;
        read_server=true;
        while (read_server)
        {
            wxInt32 id=0;
            ids.setsize(0);

            config->SetPath(s+wxString::Format(wxT("/Server/%d"),sc++));

            read_server=false;
            if (config->Read(wxT("Address"),&addr))
            {
                config->Read(wxT("Password"),&password);
                config->Read(wxT("Description"),&description);

                while (config->Read(wxString::Format(wxT("Master%d"),id++),&val))
                    ids.add(val);
                if (ids.length()==0)
                    ids.add(-1);

                if (config->Read(wxT("View"),&val))
                {
                    view=val;
                    if (config->Read(wxT("LastSeen"),&val))
                        lastSeen=(wxUint32)val;
                    if (config->Read(wxT("PlayLast"),&val))
                        playLast=(wxUint32)val;
                    if (config->Read(wxT("PlayTimeLastGame"),&val))
                        playTimeLastGame=(wxUint32)val;
                    if (config->Read(wxT("PlayTimeTotal"),&val))
                        playTimeTotal=(wxUint32)val;
                    if (config->Read(wxT("ConnectedTimes"),&val))
                        connectedTimes=(wxUint32)val;
                    read_server=true;
                }
            }

            if (!read_server)
                break;

            loopv(ids)
            {
                CslServerInfo *info=new CslServerInfo(addr,(CSL_GAMETYPE)g,view,lastSeen,
                                                      playLast,playTimeLastGame,
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

finish:
    if (config)
        delete config;

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

    wxFileConfig *config=new wxFileConfig(wxEmptyString,wxEmptyString,
                                          s+PATHDIV+wxT("servers.ini"),
                                          wxEmptyString,wxCONFIG_USE_LOCAL_FILE);
    config->SetUmask(0077);
    config->DeleteAll();

    wxInt32 mc,sc;
    CslMaster *master;
    CslServerInfo *info;
    CslGame *game;
    vector<CslMaster*> *masters;
    vector<CslServerInfo*> *servers;
    vector<CslGame*> *games=m_engine->GetGames();

    if (!games->length())
        goto finish;

    config->SetPath(wxT("/Version"));
    config->Write(wxT("Version"),CSL_SERVERCONFIG_VERSION);

    servers=m_engine->GetFavourites();

    loopv(*games)
    {
        game=games->at(i);
        s=wxT("/")+game->GetName();
        config->SetPath(s);

        masters=game->GetMasters();

        mc=0;
        loopvj(*masters)
        {
            master=masters->at(j);

            config->SetPath(s+s.Format(wxT("/Master/%d"),mc++));
            config->Write(wxT("Address"),master->GetAddress());
            config->Write(wxT("Path"),master->GetPath());
            config->Write(wxT("ID"),master->GetID());
        }

        servers=game->GetServers();
        if (servers->length()==0)
            continue;

        sc=0;
        loopvk(*servers)
        {
            info=servers->at(k);
            config->SetPath(s+s.Format(wxT("/Server/%d"),sc++));
            config->Write(wxT("Address"),info->m_host);
            config->Write(wxT("Password"),info->m_password);
            config->Write(wxT("Description"),info->m_descOld);

            loopvj(info->m_masterIDs)
            {
                config->Write(wxString::Format(wxT("Master%d"),j),info->m_masterIDs[j]);
            }
            config->Write(wxT("View"),(int)info->m_view);
            config->Write(wxT("LastSeen"),(int)info->m_lastSeen);
            config->Write(wxT("PlayLast"),(int)info->m_playLast);
            config->Write(wxT("PlayTimeLastGame"),(int)info->m_playTimeLastGame);
            config->Write(wxT("PlayTimeTotal"),(int)info->m_playTimeTotal);
            config->Write(wxT("ConnectedTimes"),(int)info->m_connectedTimes);
        }
    }

finish:
    delete config;
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
