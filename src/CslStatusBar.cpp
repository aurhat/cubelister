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

#include "CslStatusBar.h"

#include "img/green_16.xpm"
#include "img/grey_16.xpm"
#include "img/red_16.xpm"
#include "img/yellow_16.xpm"

BEGIN_EVENT_TABLE(CslStatusBar, wxStatusBar)
    EVT_SIZE(CslStatusBar::OnSize)
END_EVENT_TABLE()

enum { FIELD_LIGHT = 0, FIELD1, FIELD_END };


CslStatusBar* CslStatusBar::m_self=NULL;

CslStatusBar::CslStatusBar(wxWindow *parent) : wxStatusBar(parent, wxID_ANY), m_bmp(NULL)
{
    SetMinHeight(16);
    SetFieldsCount(FIELD_END);
    int styles[FIELD_END]={wxSB_FLAT,wxSB_FLAT};
    SetStatusStyles(FIELD_END,styles);
    m_bmp=new wxStaticBitmap(this,wxID_ANY,wxBitmap(grey_16_xpm));
}

void CslStatusBar::OnSize(wxSizeEvent& event)
{
    if (!m_bmp)
        return;

    wxRect rect;
    GetFieldRect(0,rect);
    wxSize size=m_bmp->GetSize();
#ifdef __WXGTK__
#define CSL_STATUS_IMG_OFFSET 1
#else
#define CSL_STATUS_IMG_OFFSET 0
#endif
    m_bmp->Move(rect.x+(rect.width-size.x)/2+CSL_STATUS_IMG_OFFSET,
                rect.y+(rect.height-size.y)/2+CSL_STATUS_IMG_OFFSET);

    int widths[FIELD_END]={20,-1};
    SetStatusWidths(FIELD_END,widths);

    event.Skip();
}

void CslStatusBar::SetLight(wxInt32 light)
{
    m_light=light;

    switch (light)
    {
        case LIGHT_GREEN:
            m_bmp->SetIcon(wxIcon(green_16_xpm));
            break;
        case LIGHT_GREY:
            m_bmp->SetIcon(wxIcon(grey_16_xpm));
            break;
        case LIGHT_RED:
            m_bmp->SetIcon(wxIcon(red_16_xpm));
            break;
        case LIGHT_YELLOW:
            m_bmp->SetIcon(wxIcon(yellow_16_xpm));
            break;
    }
}
