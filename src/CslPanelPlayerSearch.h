/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#ifndef CSLLISTCTRLPLAYERSEARCH_H
#define CSLLISTCTRLPLAYERSEARCH_H

#include "CslListCtrl.h"

class CslPlayerSearchEntry
{
    public:
        CslPlayerSearchEntry(CslServerInfo *info,const CslPlayerStatsData& player) :
                Info(info),Player(player) {}

        CslServerInfo *Info;
        CslPlayerStatsData Player;
};

WX_DEFINE_ARRAY(CslPlayerSearchEntry*, CslArrayCslPlayerSearchEntry);

class CslListCtrlPlayerSearch : public CslListCtrl
{
    public:
        CslListCtrlPlayerSearch(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                                const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                                const wxValidator& validator=wxDefaultValidator,
                                const wxString& name=wxListCtrlNameStr);
        ~CslListCtrlPlayerSearch();

        void ListClear();
        void AddResult(CslServerInfo *info, CslPlayerStatsData *player, bool rebuild=false);
        void RemoveServer(CslServerInfo *info);

    private:
        CslArrayCslPlayerSearchEntry m_entries;
        CslPlayerSearchEntry *m_selected;

        void OnColumnLeftClick(wxListEvent& event);
        void OnItem(wxListEvent& event);
        void OnItemDeselected(wxListEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnMenu(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        void GetToolTipText(wxInt32 row,CslToolTipEvent& event);
};

#endif //CSLLISTCTRLPLAYERSEARCH_H
