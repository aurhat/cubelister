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

#ifndef CSLLISTCTRLPLAYER_H
#define CSLLISTCTRLPLAYER_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include "CslGame.h"
#include "CslTools.h"


class CslListCtrlPlayer : public wxListCtrl
{
    public:
        CslListCtrlPlayer(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                          const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                          const wxValidator& validator=wxDefaultValidator,
                          const wxString& name=wxListCtrlNameStr);

        void ListInit(const bool smallview=false);
        void ListAdjustSize();
        void ListUpdatePlayerData(CslPlayerStats *stats,const CSL_GAMETYPE type);

    private:
        bool m_small;
        wxImageList m_imageList;
        CslListSortHelper m_sortHelper;

        void OnSize(wxSizeEvent& event);
        void OnColumnLeftClick(wxListEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        void ListSort(const wxInt32 column);
        void ToggleSortArrow();
        void ShowPanelMap(const bool show);

        static int wxCALLBACK ListSortCompareFunc(long item1,long item2,long data);
};

#endif
