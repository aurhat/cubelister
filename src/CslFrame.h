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
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include "CslMenu.h"
#include "CslStatusBar.h"
#include "CslEngine.h"
#include "CslDlgSettings.h"
#include "CslListCtrlServer.h"
#include "CslListCtrlInfo.h"
#include "CslDlgOutput.h"
#include "CslDlgTraffic.h"


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


class CslFrame: public wxFrame
{
    public:
        CslFrame(wxWindow *parent,int id,const wxString& title,
                 const wxPoint& pos=wxDefaultPosition,
                 const wxSize& size=wxDefaultSize,
                 long style=wxDEFAULT_FRAME_STYLE);
        ~CslFrame();

    private:
        CslEngine *m_engine;

        wxAuiManager m_AuiMgr;
        CslMenu *m_menu;
        wxMenu *m_menuMaster;

        wxTreeItemId m_treeGamesRoot;

        wxImageList m_imgListTree;
        wxImageList m_imgListButton;

        wxInt32 m_timerCount,m_timerUpdate;
        wxInt32 m_lightCount,m_statusCount;
        bool m_timerInit;
        wxTimer m_timer;

        CslDlgOutput *m_outputDlg;
        CslDlgExtended *m_extendedDlg;
        CslDlgTraffic *m_trafficDlg;

        CslVersionCheckThread *m_versionCheckThread;

        void set_properties();
        void do_layout();

        void OnSize(wxSizeEvent& event);
        void OnPong(wxCommandEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnListItemSelected(wxListEvent& event);
        void OnTreeLeftClick(wxTreeEvent& event);
        void OnTreeRightClick(wxTreeEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnKeypress(wxKeyEvent& event);
        void OnShow(wxShowEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnMouseEnter(wxMouseEvent& event);
        void OnMouseLeave(wxMouseEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);
        void OnVersionCheck(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        wxTreeCtrl *tree_ctrl_games;
        wxPanel *pane_games;
        CslListCtrlInfo *list_ctrl_info;
        wxPanel *pane_serverinfo;
        CslListCtrlPlayer *list_ctrl_players;
        wxPanel *pane_playerinfo;
        wxSplitterWindow *splitter_info;
        wxPanel *pane_info;
        wxSplitterWindow *splitter_games_info;
        wxTextCtrl *text_ctrl_search;
        wxPanel *panel_search;
        wxPanel *pane_main_left;
        CslListCtrlServer *list_ctrl_master;
        wxCheckBox *checkbox_filter_offline_master;
        wxCheckBox *checkbox_filter_full_master;
        wxCheckBox *checkbox_filter_empty_master;
        wxCheckBox *checkbox_filter_nonempty_master;
        wxCheckBox *checkbox_filter_mm2_master;
        wxCheckBox *checkbox_filter_mm3_master;
        wxPanel *pane_master;
        CslListCtrlServer *list_ctrl_favourites;
        wxCheckBox *checkbox_filter_offline_favourites;
        wxCheckBox *checkbox_filter_full_favourites;
        wxCheckBox *checkbox_filter_empty_favourites;
        wxCheckBox *checkbox_filter_nonempty_favourites;
        wxCheckBox *checkbox_filter_mm2_favourites;
        wxCheckBox *checkbox_filter_mm3_favourites;
        wxPanel *pane_favourites;
        wxPanel *pane_main_right;
        wxSplitterWindow *splitter_main;
        wxPanel *panel_main;

        wxFlexGridSizer *m_sizerMaster,*m_sizerFavourites;
        wxFlexGridSizer *m_sizerFilterMaster,*m_sizerFilterFavourites;
        wxFlexGridSizer *m_sizerLeft,*m_sizerSearch;
        wxBitmapButton *bitmap_button_search_clear;
        wxMenuBar *m_menubar;

        void CreateMainMenu();

        void ToggleFilter();
        void UpdateFilterCheckBoxes();
        void HandleFilterEvent(const wxInt32 id,const bool checked);

        void UpdateMaster();

        wxTreeItemId TreeFindItem(wxTreeItemId parent,CslGame *game,CslMaster *master);
        void TreeAddMaster(wxTreeItemId parent,CslMaster *master,bool activate);
        void TreeRemoveMaster();
        void TreeAddGame(CslGame *game,bool activate=false);
        void TreeCalcTotalPlaytime(CslGame *game);

        void LoadSettings();
        void SaveSettings();
        bool LoadServers(wxUint32 *numm=NULL,wxUint32 *nums=NULL);
        void SaveServers();
};



class CslApp: public wxApp
{
    public:
        bool OnInit();
        virtual int OnExit();

    private:
        wxSingleInstanceChecker *m_single;
        wxLocale m_locale;
};

#endif // CSLFRAME_H
