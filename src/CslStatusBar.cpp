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

#include "Csl.h"
#include "CslArt.h"
#include "CslStatusBar.h"

BEGIN_EVENT_TABLE(CslStatusBar, wxStatusBar)
    EVT_SIZE(CslStatusBar::OnSize)
END_EVENT_TABLE()

enum { FIELD_LIGHT = 0, FIELD1, FIELD_END };

CslStatusBar* CslStatusBar::m_self = NULL;


CslStatusBar::CslStatusBar(wxWindow *parent) :
        wxStatusBar(parent, wxID_ANY, wxST_SIZEGRIP),
        m_light(LIGHT_GREY)
{
    SetMinHeight(16);
    SetFieldsCount(FIELD_END);
    int styles[FIELD_END] = { wxSB_FLAT, wxSB_FLAT };
    SetStatusStyles(FIELD_END, styles);
    int widths[FIELD_END] = { 20, -2 };
    SetStatusWidths(FIELD_END, widths);

    m_bmp = new CslBufferedStaticBitmap(this, wxSize(16,16));
    SetLight(m_light);
}

void CslStatusBar::OnSize(wxSizeEvent& event)
{
    if (!m_self || !m_bmp)
        return;

    wxRect rect;
    GetFieldRect(0, rect);

#if defined(__WXGTK__) || defined(__WXMSW__)
#define CSL_STATUS_IMG_OFFSET 1
#else
#define CSL_STATUS_IMG_OFFSET 0
#endif

    m_bmp->Move(rect.x + (rect.width-16)/2 + CSL_STATUS_IMG_OFFSET,
                rect.y + (rect.height-16)/2 + CSL_STATUS_IMG_OFFSET);

    event.Skip();
}

void CslStatusBar::SetLight(wxInt32 light)
{
    if (m_light==light)
        return;

    m_light = light;

    static wxBitmap grey = GET_ART_MENU(wxART_BUBBLE_GREY);
    static wxBitmap green = GET_ART_MENU(wxART_BUBBLE_GREEN);
    static wxBitmap yellow = GET_ART_MENU(wxART_BUBBLE_YELLOW);
    static wxBitmap red = GET_ART_MENU(wxART_BUBBLE_RED);

    switch (light)
    {
        case LIGHT_GREY:
            m_bmp->SetBitmap(grey);
            break;
        case LIGHT_GREEN:
            m_bmp->SetBitmap(green);
            break;
        case LIGHT_YELLOW:
            m_bmp->SetBitmap(yellow);
            break;
        case LIGHT_RED:
            m_bmp->SetBitmap(red);
            break;
    }
}
