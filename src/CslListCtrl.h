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

#ifndef CSLLISTCTRL_H
#define CSLLISTCTRL_H

/**
    @author Glen Masgai <mimosius@users.sourceforge.net>
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
#include "CslToolTip.h"


enum
{
    CSL_LIST_IMG_SORT_ASC = 0,
    CSL_LIST_IMG_SORT_DSC,
    CSL_LIST_IMG_UNKNOWN
};


class CslListCtrl : public wxListCtrl
{
    public:
        CslListCtrl(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                    const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                    const wxValidator& validator=wxDefaultValidator,
                    const wxString& name=wxListCtrlNameStr);
        ~CslListCtrl();

        void CreateScreenShot();
        wxUint32 GetCountryFlag(wxUint32 ip);
        static void CreateCountryFlagImageList();

    private:
        wxUint32 m_mouseLastMove;

        bool m_flickerFree;

#ifdef __WXMSW__
        void OnEraseBackground(wxEraseEvent& event);
#endif
        void OnMouseMove(wxMouseEvent& event);
        void OnItem(wxListEvent& event);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnToolTip(CslToolTipEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        static wxImageList ListImageList;

        void FlickerFree(bool val) { m_flickerFree=val; }
        virtual wxWindow *GetScreenShotWindow() { return this; }
        virtual wxString GetScreenShotFileName();
        virtual void GetToolTipText(wxInt32 row,CslToolTipEvent& event) {}
        virtual wxSize GetImageListSize() { return wxDefaultSize; }
};

#endif //CSLLISTCTRL_H
