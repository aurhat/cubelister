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

#include "CslToolTip.h"
#include "CslSettings.h"


DEFINE_EVENT_TYPE(wxCSL_EVT_TOOLTIP)

BEGIN_EVENT_TABLE(CslToolTip,wxFrame)
    #ifdef __WXMSW__
    EVT_ERASE_BACKGROUND(CslToolTip::OnEraseBackground)
    #else
    EVT_PAINT(CslToolTip::OnPaint)
    #endif
    EVT_LEAVE_WINDOW(CslToolTip::OnMouseLeave)
    EVT_LEFT_DOWN(CslToolTip::OnMouseButton)
    EVT_RIGHT_DOWN(CslToolTip::OnMouseButton)
    EVT_MIDDLE_DOWN(CslToolTip::OnMouseButton)
    EVT_MOUSEWHEEL(CslToolTip::OnMouseButton)
    EVT_TIMER(wxID_ANY,CslToolTip::OnTimer)
END_EVENT_TABLE()


CslToolTip *CslToolTip::m_self=NULL;

CslToolTip::CslToolTip(wxWindow *parent) :
        wxFrame(parent,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,
                wxNO_BORDER|wxSTAY_ON_TOP|wxFRAME_TOOL_WINDOW),
        m_current(NULL)
{
    m_self=this;
    m_timer.SetOwner(this);

    m_sizer=new wxBoxSizer(wxVERTICAL);
    m_title=new wxStaticText(this,wxID_ANY,wxEmptyString);
    m_sizer->Add(m_title,0,wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL,6);
    wxBoxSizer *box=new wxBoxSizer(wxHORIZONTAL);
    m_sizer->Add(box,1,wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL,4);
    m_left=new wxStaticText(this,wxID_ANY,wxEmptyString);
    m_right=new wxStaticText(this,wxID_ANY,wxEmptyString);
    box->Add(m_left,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,6);
    box->Add(m_right,1,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,6);
    SetSizer(m_sizer);

    wxFont font=m_title->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_left->SetFont(font);
    m_title->SetFont(font);

    m_title->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_left->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_right->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_title->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_left->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_right->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_title->Connect(wxEVT_MIDDLE_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_left->Connect(wxEVT_MIDDLE_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_right->Connect(wxEVT_MIDDLE_DOWN,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_title->Connect(wxEVT_MOUSEWHEEL,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_left->Connect(wxEVT_MOUSEWHEEL,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
    m_right->Connect(wxEVT_MOUSEWHEEL,wxMouseEventHandler(CslToolTip::OnMouseButton),NULL,this);
}

CslToolTip::~CslToolTip()
{
    ResetTip();
    m_self=NULL;
}

#ifdef __WXMSW__
void CslToolTip::OnEraseBackground(wxEraseEvent& event)
{
    wxDC& dc=*event.GetDC();
#else
void CslToolTip::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
#endif
    wxInt32 w=0,h=0;
    GetClientSize(&w,&h);

#ifdef __WXMAC__
    dc.SetPen(*wxTRANSPARENT_PEN);
#else
    wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    dc.SetPen(pen);
#endif

    wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
    dc.SetBrush(brush);

    dc.Clear();
#ifdef __WXMAC__
    dc.DrawRectangle(0,0,w,h);
#else
    dc.DrawRoundedRectangle(0,0,w,h,3.0);
#endif
}

void CslToolTip::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (!m_current)
        return;

    CslToolTipEvent evt(::wxGetMousePosition());

    if (m_current->ProcessEvent(evt))
        ShowTip(evt);
}

void CslToolTip::OnMouseLeave(wxMouseEvent& event)
{
    if (event.GetEventObject()==this)
    {
        const wxSize& size=GetClientSize();
        const wxPoint& pos=event.GetPosition();

        if (pos.x>=size.x || pos.y>=size.y || pos.x<=0 || pos.y<=0)
            Hide();
    }
    else if (!IsShown())
        ResetTip();

    event.Skip();
}

void CslToolTip::OnMouseButton(wxMouseEvent& event)
{
    Hide();

    event.Skip();
}

void CslToolTip::ShowTip(CslToolTipEvent& event)
{
    wxUint32 i;
    wxString left,right;

#ifdef __WXMSW__
    //set the default tooltip bg colour otherwise it's the default window bg colour
    const wxColour& bg=wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
    m_left->SetBackgroundColour(bg);
    m_right->SetBackgroundColour(bg);
    m_title->SetBackgroundColour(bg);
#endif

    event.Title.Replace(wxT("&"),wxT("&&"));

    for (i=0;i<event.Text.GetCount();i++)
    {
        event.Text.Item(i).Replace(wxT("&"),wxT("&&"));

        if (i%2==0)
            left<<wxT("\n")<<event.Text.Item(i);
        else
            right<<wxT("\n")<<event.Text.Item(i);
    }

    m_left->SetLabel(left);
    m_right->SetLabel(right);
    m_title->SetLabel(event.Title);

    if (event.Title.IsEmpty() && m_title->IsShown())
        m_sizer->Hide(m_title);
    else if (!event.Title.IsEmpty() && !m_title->IsShown())
        m_sizer->Show(m_title);

#ifdef __WXMAC__
    {
        wxClientDC dc(m_left);
        const wxSize& size=dc.GetMultiLineTextExtent(left);
        m_left->SetClientSize(size);
    }
    {
        wxClientDC dc(m_right);
        const wxSize& size=dc.GetMultiLineTextExtent(right);
        m_right->SetClientSize(size);
    }
#endif

    m_sizer->Fit(this);

#ifndef __WXGTK__
    // ensure the tip is fully shown on the screen
    const wxRect& client=GetRect();
    const wxRect& screen=::wxGetClientDisplayRect();

    if (event.Pos.x+client.width>screen.width)
        event.Pos.x-=(event.Pos.x+client.width-screen.width);
    if (event.Pos.y+client.height>screen.height)
        event.Pos.y-=(event.Pos.y+client.height-screen.height);
#endif

    Move(event.Pos);
    Show();
}

void CslToolTip::InitTip(wxEvtHandler *handler)
{
    if (!m_self)
        return;

    ResetTip();

    if (handler)
    {
        m_self->m_current=handler;
        m_self->m_current->Connect(wxEVT_LEAVE_WINDOW,wxMouseEventHandler(CslToolTip::OnMouseLeave),NULL,m_self);
        m_self->m_timer.Start(g_cslSettings->tooltipDelay,wxTIMER_ONE_SHOT);
    }
}

void CslToolTip::ResetTip()
{
    if (!m_self)
        return;

    if (m_self->m_current)
    {
        m_self->m_current->Disconnect(wxEVT_LEAVE_WINDOW,wxMouseEventHandler(CslToolTip::OnMouseLeave),NULL,m_self);
        m_self->m_current=NULL;
    }

    if (m_self->IsShown())
        m_self->Hide();
    if (m_self->m_timer.IsRunning())
        m_self->m_timer.Stop();
}
