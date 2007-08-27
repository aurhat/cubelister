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
#include <wx/image.h>
#include <wx/imaglist.h>
// begin wxGlade: ::dependencies
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
// end wxGlade
#include "CslStatusBar.h"
#include "CslEngine.h"
#include "CslDlgSettings.h"
#include "CslListCtrlServer.h"
#include "CslListCtrlInfo.h"
#include "CslDlgOutput.h"

class CslFrame: public wxFrame
{
    public:
        // begin wxGlade: CslFrame::ids
        // end wxGlade

        CslFrame(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition,
                 const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);
        ~CslFrame();

    private:
        CslEngine *m_engine;

        wxMenu *m_menuMaster;

        wxColour m_textctrlText,m_textctrlBack;

        wxTreeItemId m_treeGamesRoot;

        wxImageList m_imgListTree;
        wxImageList m_imgListButton;

        wxInt32 m_timerCount,m_timerUpdate;
        wxInt32 m_lightCount,m_statusCount;
        bool m_timerInit;
        wxTimer m_timer;

         // TODO Output from Game
        //CslDlgOutput *m_outputDlg;

        // begin wxGlade: CslFrame::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnTimer(wxTimerEvent& event);
        void OnTreeLeftClick(wxTreeEvent& event);
        void OnTreeRightClick(wxTreeEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnShow(wxShowEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnMouseEnter(wxMouseEvent& event);
        void OnMouseLeave(wxMouseEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslFrame::attributes
        wxStaticBox* sizer_filter_staticbox;
        wxMenuBar* frame_csl_menubar;
        wxTreeCtrl* tree_ctrl_games;
        wxPanel* pane_games;
        CslListCtrlInfo* list_ctrl_info;
        wxPanel* pane_info;
        wxSplitterWindow* splitter_games_info;
        wxCheckBox* checkbox_filter_full;
        wxCheckBox* checkbox_filter_offline;
        wxCheckBox* checkbox_filter_empty;
        wxCheckBox* checkbox_filter_mm2;
        wxCheckBox* checkbox_filter_nonempty;
        wxCheckBox* checkbox_filter_mm3;
        wxCheckBox* checkbox_filter_favourites;
        wxButton* button_filter_reset;
        wxPanel* pane_main_left;
        CslListCtrlServer* list_ctrl_master;
        wxTextCtrl* text_ctrl_search;
        wxPanel* panel_search;
        wxPanel* pane_master;
        CslListCtrlServer* list_ctrl_favourites;
        wxPanel* pane_favourites;
        wxSplitterWindow* splitter_lists;
        wxPanel* pane_main_right;
        wxSplitterWindow* splitter_main;
        wxPanel* panel_main;
        wxPanel* panel_frame;
        // end wxGlade
        wxBitmapButton* bitmap_button_search_clear;
        wxFlexGridSizer *m_sizerMaster,*m_sizerLeft,*m_sizerSearch;
        wxStaticBoxSizer *m_sizerFilter;

        void ToggleSearchBar();
        void ToggleFilter();
        void ToggleSplitterUpdate();
        void UpdateFilterCheckBoxes();

        void UpdateMaster();

        wxTreeItemId TreeFindItem(wxTreeItemId parent,CslGame *game,CslMaster *master);
        void TreeAddMaster(wxTreeItemId parent,CslMaster *master,bool activate);
        void TreeRemoveMaster();
        void TreeAddGame(CslGame *game,bool activate=false);

        void LoadSettings();
        void SaveSettings();
        bool LoadServers(wxUint32 *numm=NULL,wxUint32 *nums=NULL);
        void SaveServers();
}
; // wxGlade: end class



class CslApp: public wxApp
{
    public:
        bool OnInit();

    private:
        wxLocale m_locale;
};

#endif // CSLFRAME_H
