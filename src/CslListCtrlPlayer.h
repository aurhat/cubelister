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

#ifndef CSLLISTCTRLPLAYER_H
#define CSLLISTCTRLPLAYER_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
*/

#include "CslListCtrl.h"
#include "engine/CslGame.h"
#include "engine/CslTools.h"


class CslListCtrlPlayer : public CslListCtrl
{
    public:
        enum
        {
            SIZE_MICRO, SIZE_MINI,
            SIZE_DEFAULT, SIZE_FULL
        };

        CslListCtrlPlayer(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                          const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                          const wxValidator& validator=wxDefaultValidator,
                          const wxString& name=wxListCtrlNameStr);

        void ListInit(const wxInt32 view);
        void ListAdjustSize(const wxSize& size=wxDefaultSize);
        void UpdateData();
        void EnableEntries(bool enable);

        void ServerInfo(CslServerInfo *info);
        CslServerInfo* ServerInfo() { return m_info; }
        wxInt32 View() { return m_view; }

        static wxSize BestSizeMicro;
        static wxSize BestSizeMini;

    private:
        wxInt32 m_view;
        CslServerInfo *m_info;

        CslListSortHelper m_sortHelper;

        void OnColumnLeftClick(wxListEvent& event);
        void OnItemActivated(wxListEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnMenu(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        void ListSort(const wxInt32 column);

        void GetToolTipText(wxInt32 row,CslToolTipEvent& event);
        wxString GetScreenShotFileName();
        wxWindow *GetScreenShotWindow() { return GetParent(); }
        wxSize GetImageListSize();


        static int wxCALLBACK ListSortCompareFunc(long item1,long item2,long data);
};


class CslPanelPlayer : public wxPanel
{
    public:
        CslPanelPlayer(wxWindow* parent,long listStyle=wxLC_ICON);

        CslListCtrlPlayer* ListCtrl() { return m_listCtrl; }
        CslServerInfo* ServerInfo() { return m_listCtrl->ServerInfo(); }
        void ServerInfo(CslServerInfo *info) { m_listCtrl->ServerInfo(info); }
        void UpdateData();
        void CheckServerStatus();

    private:
        void OnSize(wxSizeEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        wxFlexGridSizer *m_sizer;
        CslListCtrlPlayer *m_listCtrl;
        wxStaticText *m_label;

        wxString GetLabelText();
};

#endif
