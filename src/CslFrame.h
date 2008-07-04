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

#ifndef CSLFRAME_H
#define CSLFRAME_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/aui/aui.h"
#include <wx/snglinst.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include "engine/CslEngine.h"
#include "CslMenu.h"
#include "CslDlgExtended.h"
#include "CslStatusBar.h"
#include "CslDlgSettings.h"
#include "CslListCtrlServer.h"
#include "CslListCtrlInfo.h"
#include "CslDlgOutput.h"
#include "CslDlgTraffic.h"
#include "CslListCtrlPlayer.h"


class CslVersionCheckThread : public wxThread
{
    public:
        CslVersionCheckThread(wxEvtHandler *evtHandler) :
                wxThread(wxTHREAD_JOINABLE),m_evtHandler(evtHandler)
        {
            Create();
        }

        virtual ExitCode Entry();

    private:
        wxEvtHandler *m_evtHandler;
};


class CslPlayerInfo : public wxPanel
{
    public:
        CslPlayerInfo(wxWindow* parent,long listStyle=wxLC_ICON);

        CslListCtrlPlayer* ListCtrl() { return m_listCtrl; }
        CslServerInfo* ServerInfo() { return m_listCtrl->ServerInfo(); }
        void ServerInfo(CslServerInfo *info) { m_listCtrl->ServerInfo(info); }

        void UpdateData();

    private:
        wxFlexGridSizer *m_sizer;
        CslListCtrlPlayer *m_listCtrl;
        wxStaticText *m_label;
};


class CslFrame: public wxFrame
{
    public:
        CslFrame(wxWindow *parent,int id,const wxString& title,
                 const wxPoint& pos=wxDefaultPosition,
                 const wxSize& size=wxDefaultSize,
                 long style=wxDEFAULT_FRAME_STYLE);
        ~CslFrame();

    protected:
        wxAuiManager m_AuiMgr;

        wxFlexGridSizer *sizer_main,*sizer_search;
        wxPanel *pane_main,*pane_search;
        CslListCtrlServer *list_ctrl_master,*list_ctrl_favourites;
        CslPlayerInfo *player_info;
        CslListCtrlInfo *list_ctrl_info;
        wxTreeCtrl *tree_ctrl_games;
        wxTextCtrl *text_ctrl_search;
        wxStaticText *text_search_result;
        wxButton *button_search;
        wxBitmapButton *button_search_clear;
        wxGauge *gauge_search;
        wxRadioButton *radio_search_server,*radio_search_player;
        wxMenuBar *menubar;
        wxMenu *menuMaster;

        wxString m_defaultLayout;

        CslMenu *m_menu;
        wxImageList m_imgListTree;
        wxImageList m_imgListButton;
        wxTreeItemId m_treeGamesRoot;

        wxTimer m_timer;
        wxInt32 m_timerCount,m_timerUpdate;
        bool m_timerInit;

        CslEngine *m_engine;

        CslDlgOutput *m_outputDlg;
        CslDlgExtended *m_extendedDlg;
        CslDlgTraffic *m_trafficDlg;

        CslVersionCheckThread *m_versionCheckThread;

        vector<CslPlayerInfo*> m_playerInfos;

        CslServerInfo *m_oldSelectedInfo;

        vector<CslServerInfo*> m_searchedServers;
        wxString m_searchString;
        wxInt32 m_searchResultPlayer,m_searchResultServer;

        void CreateMainMenu();
        void CreateControls();
        void SetProperties();
        void DoLayout();

        wxString PlayerListGetCaption(CslServerInfo *info,bool selected);
        void PlayerListCreateView(CslServerInfo *info,wxUint32 view,const wxString& name=wxEmptyString);

        void ToggleSearchBar();
        void TogglePane(wxInt32 id);

        void SetTotalPlaytime(CslGame *game);
        void SetListCaption(wxInt32 id,const wxString& addon=wxEmptyString);
        void SetSearchbarColour(bool value);

        wxTreeItemId TreeFindItem(wxTreeItemId parent,CslGame *game,CslMaster *master);
        void TreeAddMaster(wxTreeItemId parent,CslMaster *master,bool focus=false);
        void TreeRemoveMaster();
        void TreeAddGame(CslGame *game,wxInt32 img,bool focus=false);
        CslGame* TreeGetSelectedGame(wxTreeItemId *item=NULL);
        CslMaster *TreeGetSelectedMaster(wxTreeItemId *item=NULL);

        void UpdateMaster();
        void ConnectToServer();

        void LoadSettings();
        void SaveSettings();
        bool LoadServers(wxUint32 *numm=NULL,wxUint32 *nums=NULL);
        void SaveServers();

    private:
        void OnPong(wxCommandEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnListItemSelected(wxListEvent& event);
        void OnListItemActivated(wxListEvent& event);
        void OnTreeSelChanged(wxTreeEvent& event);
        void OnTreeRightClick(wxTreeEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnKeypress(wxKeyEvent& event);
        void OnShow(wxShowEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnPaneClose(wxAuiManagerEvent& event);
        void OnMouseEnter(wxMouseEvent& event);
        void OnMouseLeave(wxMouseEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);
        void OnVersionCheck(wxCommandEvent& event);
        void OnEndProcess(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()
};



class CslApp: public wxApp
{
    public:
        bool OnInit();
        int OnExit();

    private:
        wxSingleInstanceChecker *m_single;
        wxLocale m_locale;
};

#endif // CSLFRAME_H
