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

#ifndef CSLLISTCTRL_H
#define CSLLISTCTRL_H

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


enum
{
    CSL_LIST_IMG_SORT_ASC = 0,
    CSL_LIST_IMG_SORT_DSC,
    CSL_LIST_IMG_UNKNOWN
};


class CslToolTip : public wxFrame
{
    public:
        CslToolTip(wxWindow *parent);

        void ShowTip(const wxString& title,const wxArrayString& text,const wxPoint& position);

    private:
        wxBoxSizer *m_sizer;
        wxStaticText *m_title,*m_left,*m_right;

        void OnPaint(wxPaintEvent& event);
        void OnMouse(wxMouseEvent& event);

        DECLARE_EVENT_TABLE();
};


class CslListCtrl : public wxListCtrl
{
    public:
        CslListCtrl(wxWindow* parent,wxWindowID id,const wxPoint& pos=wxDefaultPosition,
                    const wxSize& size=wxDefaultSize,long style=wxLC_ICON,
                    const wxValidator& validator=wxDefaultValidator,
                    const wxString& name=wxListCtrlNameStr);
        ~CslListCtrl();

        wxUint32 GetCountryFlag(wxUint32 ip);
        static void CreateCountryFlagImageList();

    private:
        wxTimer m_timer;

        wxUint32 m_mouseLastMove;

        CslToolTip *m_toolTip;
        wxString m_toolTipTitle;
        wxArrayString m_toolTipText;

        void OnMouseMove(wxMouseEvent& event);
        void OnMouseLeave(wxMouseEvent& event);
        void OnTimer(wxTimerEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        static wxImageList ListImageList;

        virtual void GetToolTipText(wxInt32 row,wxString& title,wxArrayString& text) {}
};

#endif //CSLLISTCTRL_H
