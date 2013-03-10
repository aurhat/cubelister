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

#ifndef CSLLISTCTRLPLAYER_H
#define CSLLISTCTRLPLAYER_H

#include "CslListCtrl.h"

class CslListCtrlPlayerView : public CslListCtrl
{
    public:
        enum { SIZE_MICRO, SIZE_MINI, SIZE_DEFAULT, SIZE_FULL };

        CslListCtrlPlayerView(wxWindow* parent = NULL,
                              wxWindowID id = wxID_ANY,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxLC_REPORT,
                              const wxValidator& validator = wxDefaultValidator,
                              const wxString& name = wxListCtrlNameStr);
                              // to save columns in settings it needs a unique name
        ~CslListCtrlPlayerView();

        void ListInit(wxInt32 view);
        void UpdateData();
        void EnableEntries(bool enable);
        void ListClear();

        void SetServerInfo(CslServerInfo *info);
        CslServerInfo* GetServerInfo()
        {
            return m_info;
        }
        wxInt32 View()
        {
            return m_view;
        }

        static wxSize BestSizeMicro;
        static wxSize BestSizeMini;

    protected:
        // CslListCtrl virtual functions
        virtual bool ListFindItemCompare(void *data1, void *data2)
        {
            return *(CslPlayerStatsData*)data1==*(CslPlayerStatsData*)data2;
        }
        void GetToolTipText(wxInt32 row,CslToolTipEvent& event);
        wxString GetScreenShotFileName();
        wxWindow *GetScreenShotWindow()
        {
            return GetParent();
        }

        static int wxCALLBACK ListSortCompareFunc(long item1, long item2, long data);

    private:
        wxString& FormatStats(wxString& in, CslPlayerStatsData *data, int type);

        void OnItemSelected(wxListEvent& event);
        void OnColumnToggle(wxListEvent& event);
        void OnItemDeselected(wxListEvent& event);
        void OnItemActivated(wxListEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnMenu(wxCommandEvent& event);

        wxInt32 m_view;
        CslServerInfo *m_info;

        DECLARE_EVENT_TABLE()
        DECLARE_DYNAMIC_CLASS(CslListCtrlPlayerView)
        DECLARE_NO_COPY_CLASS(CslListCtrlPlayerView)
};


class CslPanelServerView : public wxPanel
{
    private:
        DECLARE_DYNAMIC_CLASS(CslPanelServerView)

        CslPanelServerView() { }

    public:
        CslPanelServerView(wxWindow* parent,
                           const wxString& listname,
                           long liststyle = wxLC_REPORT);

        CslListCtrlPlayerView* ListCtrl()
        {
            return m_list_ctrl;
        }
        CslServerInfo* GetServerInfo()
        {
            return m_list_ctrl->GetServerInfo();
        }
        void SetServerInfo(CslServerInfo *info)
        {
            m_list_ctrl->SetServerInfo(info);
        }

        void UpdateData();
        void CheckServerStatus();

    private:
        void OnSize(wxSizeEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        wxStaticText *m_label;
        wxFlexGridSizer *m_sizer;
        CslListCtrlPlayerView *m_list_ctrl;

        wxString GetLabelText();
};

WX_DEFINE_ARRAY(CslPanelServerView*, CslArrayCslServerView);

#endif //CSLLISTCTRLPLAYER_H
