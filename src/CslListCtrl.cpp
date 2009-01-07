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

#include "CslListCtrl.h"
#include "CslSettings.h"
#include "engine/CslTools.h"
#include "CslGeoIP.h"
#include "CslFlags.h"
#include "img/sortasc_18_12.xpm"
#include "img/sortdsc_18_12.xpm"


BEGIN_EVENT_TABLE(CslToolTip,wxFrame)
    EVT_PAINT(CslToolTip::OnPaint)
    EVT_LEFT_DOWN(CslToolTip::OnMouse)
    EVT_RIGHT_DOWN(CslToolTip::OnMouse)
    EVT_LEAVE_WINDOW(CslToolTip::OnMouse)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(CslListCtrl,wxListCtrl)
    EVT_MOTION(CslListCtrl::OnMouseMove)
    EVT_LEAVE_WINDOW(CslListCtrl::OnMouseLeave)
    EVT_TIMER(wxID_ANY,CslListCtrl::OnTimer)
END_EVENT_TABLE()


CslToolTip::CslToolTip(wxWindow *parent) :
        wxFrame(parent,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,
                wxNO_BORDER|wxSTAY_ON_TOP|wxFRAME_SHAPED|wxFRAME_NO_TASKBAR)
{
    m_sizer=new wxBoxSizer(wxVERTICAL);
    m_title=new wxStaticText(this,wxID_ANY,wxEmptyString);
    m_sizer->Add(m_title,0,wxALIGN_CENTER_HORIZONTAL|wxTOP,6);
    wxBoxSizer *box=new wxBoxSizer(wxHORIZONTAL);
    m_sizer->Add(box,1,wxEXPAND|wxLEFT|wxRIGHT,4);
    m_left=new wxStaticText(this,wxID_ANY,wxEmptyString);
    m_right=new wxStaticText(this,wxID_ANY,wxEmptyString);
    box->Add(m_left,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,6);
    box->Add(m_right,1,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,6);
    SetSizer(m_sizer);

    wxFont font=m_title->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_left->SetFont(font);
    m_title->SetFont(font);

    m_title->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CslToolTip::OnMouse),NULL,this);
    m_left->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CslToolTip::OnMouse),NULL,this);
    m_right->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CslToolTip::OnMouse),NULL,this);
    m_title->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CslToolTip::OnMouse),NULL,this);
    m_left->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CslToolTip::OnMouse),NULL,this);
    m_right->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CslToolTip::OnMouse),NULL,this);
}

void CslToolTip::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxInt32 w=0,h=0;
    GetClientSize(&w,&h);

    wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    dc.SetPen(pen);

    wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
    dc.SetBrush(brush);

    dc.Clear();
    dc.DrawRoundedRectangle(0,0,w,h,4.0);
}

void CslToolTip::OnMouse(wxMouseEvent& event)
{
    Hide();

    event.Skip();
}

void CslToolTip::ShowTip(const wxString& title,const wxArrayString& text,const wxPoint& position)
{
    wxUint32 i=0;
    wxString left,right;

    for (i=0;i<text.GetCount();i++)
    {
        if (i%2==0)
            left << wxT("\r\n") << text.Item(i) << wxT(":");
        else
            right << wxT("\r\n") << text.Item(i);
    }

    m_left->SetLabel(left);
    m_right->SetLabel(right);
    m_title->SetLabel(title);

    m_sizer->SetSizeHints(this);
    Move(position);
    Show();
}


wxImageList CslListCtrl::ListImageList;
#ifdef __WXMSW__
wxInt32 CslListCtrl::m_imgOffsetY=0;
#endif

CslListCtrl::CslListCtrl(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size,
                         long style,const wxValidator& validator,const wxString& name) :
        wxListCtrl(parent,id,pos,size,style,validator,name),
        m_mouseLastMove(0)
{
    m_toolTip=new CslToolTip(this);
    m_timer.SetOwner(this);
}

CslListCtrl::~CslListCtrl()
{
    delete m_toolTip;
}

void CslListCtrl::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();

    if (m_toolTip->IsShown())
        m_toolTip->Hide();

    if (!g_cslSettings->tooltipDelay)
        return;

    wxUint32 ticks=GetTicks();

    if (ticks-m_mouseLastMove<CSL_TOOLTIP_DELAY_STEP)
        return;

    m_mouseLastMove=ticks;

    m_timer.Start(g_cslSettings->tooltipDelay,wxTIMER_ONE_SHOT);
}

void CslListCtrl::OnMouseLeave(wxMouseEvent& event)
{
    if (m_timer.IsRunning())
        m_timer.Stop();

    event.Skip();
}

void CslListCtrl::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    wxRect rect;
    wxListItem item;
    wxInt32 i,offset=0;
#ifndef __WXMSW__
    bool first=true;
#endif
    wxPoint spos=wxGetMousePosition();
    wxPoint wpos=ScreenToClient(spos);

    if (wpos==wxDefaultPosition)
        return;

    for (i=GetTopItem();i<GetItemCount();i++)
    {
        item.SetId(i);
        GetItemRect(item,rect,wxLIST_RECT_BOUNDS);

#ifndef __WXMSW__
        if (first)
        {
            offset=rect.y;
            first=false;
        }
#endif
        rect.y-=offset;

        if (!rect.Contains(wpos))
            continue;

        m_toolTipText.Empty();
        m_toolTipTitle.Empty();

        GetToolTipText(i,m_toolTipTitle,m_toolTipText);

        if (m_toolTip->IsShown())
            m_toolTip->Hide();

        if (!m_toolTipText.IsEmpty())
            m_toolTip->ShowTip(m_toolTipTitle,m_toolTipText,spos);

        break;
    }
}

wxUint32 CslListCtrl::GetCountryFlag(wxUint32 ip)
{
    wxUint32 i;
    const char *country;

    if (ip && (country=CslGeoIP::GetCountryCodeByIPnum(ip)))
    {
        for (i=sizeof(codes)/sizeof(codes[0])-1;i>=0;i--)
        {
            if (!strcasecmp(country,codes[i]))
                return CSL_LIST_IMG_UNKNOWN+i+1;
        }
    }

    return CSL_LIST_IMG_UNKNOWN;
}

void CslListCtrl::CreateCountryFlagImageList()
{
#ifdef __WXMSW__
    //detect vista and set the offset fot the flag images to 2
    //so the region should be correct drawing the background
    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);

    if (ovi.dwMajorVersion>5)
        m_imgOffsetY=2;

    ListImageList.Create(20,14,true);
    ListImageList.Add(AdjustIconSize(sortasc_18_12_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,0)));
    ListImageList.Add(AdjustIconSize(sortdsc_18_12_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,0)));
    ListImageList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,2)));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        ListImageList.Add(AdjustIconSize(flags[i],wxNullIcon,wxSize(20,14),wxPoint(0,2)));
#else
    ListImageList.Create(18,12,true);
    ListImageList.Add(wxBitmap(sortasc_18_12_xpm));
    ListImageList.Add(wxBitmap(sortdsc_18_12_xpm));
    ListImageList.Add(wxBitmap(unknown_xpm));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        ListImageList.Add(wxBitmap(flags[i]));
#endif
}
