/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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
#include "CslToolTip.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_TOOLTIP)


BEGIN_EVENT_TABLE(CslToolTip, wxEvtHandler)
    EVT_TIMER(wxID_ANY, CslToolTip::OnTimer)
END_EVENT_TABLE()


CslToolTip::CslToolTip() :
        m_top(false), m_frame(NULL), m_parent(NULL)
{
    m_timer.SetOwner(this);
}

CslToolTip::~CslToolTip()
{
    ResetTip();
}

#ifdef __WXMSW__
void CslToolTip::OnEraseBackground(wxEraseEvent& event)
{
    if (!m_frame)
        return;

    wxDC& dc=*event.GetDC();
#else
void CslToolTip::OnPaint(wxPaintEvent& event)
{
    if (!m_frame)
        return;

    wxPaintDC dc(m_frame);
#endif
    wxInt32 w=0, h=0;
    m_frame->GetClientSize(&w, &h);

#ifdef __WXMAC__
    dc.SetPen(*wxTRANSPARENT_PEN);
#else
    dc.SetPen(wxPen(SYSCOLOUR(wxSYS_COLOUR_INFOTEXT)));
#endif
    dc.SetBrush(wxBrush(SYSCOLOUR(wxSYS_COLOUR_INFOBK)));

    dc.Clear();
#ifdef __WXMAC__
    dc.DrawRectangle(0, 0, w, h);
#else
    dc.DrawRoundedRectangle(0, 0, w, h, 2.0);
#endif
}

void CslToolTip::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (!m_parent)
        return;

    if (m_top)
    {
        ResetTip();
        return;
    }

    CreateFrame();
}

void CslToolTip::OnMouseLeave(wxMouseEvent& event)
{
    if (m_frame && event.GetEventObject()==m_frame)
    {
        const wxSize& size=m_frame->GetClientSize();
        const wxPoint& pos=event.GetPosition();

        if (pos.x>=size.x || pos.y>=size.y || pos.x<=0 || pos.y<=0)
            ResetTip();
    }

    event.Skip();
}

void CslToolTip::OnMouseButton(wxMouseEvent& event)
{
    ResetTip();

    event.Skip();
}

CslToolTip& CslToolTip::GetInstance()
{
    static CslToolTip tip;

    return tip;
}

void CslToolTip::CreateFrame()
{
    CslToolTipEvent event(::wxGetMousePosition());

    if (!m_parent->GetEventHandler()->ProcessEvent(event))
        return;

    m_frame=new wxFrame(m_parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                        wxNO_BORDER|wxFRAME_TOOL_WINDOW|(m_top ? wxSTAY_ON_TOP:wxFRAME_FLOAT_ON_PARENT));

    wxStaticText *title=new wxStaticText(m_frame, wxID_ANY, wxEmptyString);
    wxStaticText *left=new wxStaticText(m_frame, wxID_ANY, wxEmptyString);
    wxStaticText *right=new wxStaticText(m_frame, wxID_ANY, wxEmptyString);
    wxBoxSizer *box=new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer *grid=new wxFlexGridSizer(1, 2, 0, 0);
    box->Add(title, 0, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL, 6);
    box->Add(grid, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL, 4);
    grid->Add(left, 0, wxEXPAND|wxBOTTOM|wxLEFT, 6);
    grid->Add(right, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 6);
    grid->AddGrowableCol(0);
    grid->AddGrowableCol(1);

    m_frame->SetSizer(box);

    wxFont font=title->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    left->SetFont(font);
    title->SetFont(font);

#ifdef __WXMSW__
    //set the default tooltip bg colour otherwise it's the default window bg colour
    const wxColour& bg=SYSCOLOUR(wxSYS_COLOUR_INFOBK);
    left->SetBackgroundColour(bg);
    right->SetBackgroundColour(bg);
    title->SetBackgroundColour(bg);
#endif

    wxUint32 i;
    wxString l, r;

    event.Title.Replace(wxT("&"), wxT("&&"));

    for (i=0; i<event.Text.GetCount(); i++)
    {
        event.Text.Item(i).Replace(wxT("&"), wxT("&&"));

        if (i%2==0)
            l<<wxT("\n")<<event.Text.Item(i);
        else
            r<<wxT("\n")<<event.Text.Item(i);
    }

    left->SetLabel(l);
    right->SetLabel(r);
    title->SetLabel(event.Title);

    if (event.Title.IsEmpty())
        box->Hide(title);

    {
        wxClientDC dc(m_frame);
        wxInt32 gap=dc.GetTextExtent(wxT(" ")).x;
        grid->SetHGap(m_top ? gap:gap*4);
    }

#ifdef __WXMAC__
    {
        wxClientDC dc(left);
        const wxSize& size=dc.GetMultiLineTextExtent(l);
        left->SetClientSize(size);
    }
    {
        wxClientDC dc(right);
        const wxSize& size=dc.GetMultiLineTextExtent(r);
        right->SetClientSize(size);
    }
#endif

    box->Fit(m_frame);

    RegisterEventsRecursively(wxID_ANY, m_frame, this, wxEVT_LEFT_DOWN,
                              wxMouseEventHandler(CslToolTip::OnMouseButton));
    RegisterEventsRecursively(wxID_ANY, m_frame, this, wxEVT_RIGHT_DOWN,
                              wxMouseEventHandler(CslToolTip::OnMouseButton));
    RegisterEventsRecursively(wxID_ANY, m_frame, this, wxEVT_MIDDLE_DOWN,
                              wxMouseEventHandler(CslToolTip::OnMouseButton));
    RegisterEventsRecursively(wxID_ANY, m_frame, this, wxEVT_MOUSEWHEEL,
                              wxMouseEventHandler(CslToolTip::OnMouseButton));

    m_frame->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(CslToolTip::OnMouseLeave), NULL, this);
#ifdef __WXMSW__
    m_frame->Connect(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(CslToolTip::OnEraseBackground), NULL, this);
#else
    m_frame->Connect(wxEVT_PAINT, wxPaintEventHandler(CslToolTip::OnPaint), NULL, this);
#endif

#ifndef __WXGTK__
    // ensure the tip is fully shown on the screen
    const wxRect& client=m_frame->GetRect();
    const wxRect& screen=::wxGetClientDisplayRect();

    if (event.Pos.x+client.width>screen.width)
        event.Pos.x-=(event.Pos.x+client.width-screen.width);
    if (event.Pos.y+client.height>screen.height)
        event.Pos.y-=(event.Pos.y+client.height-screen.height);
#endif

    m_frame->Move(event.Pos);
    m_frame->Show();

    if (m_top)
        m_timer.Start(10000, wxTIMER_ONE_SHOT);
}

void CslToolTip::InitTip(wxWindow *window, wxInt32 delay, bool top)
{
    CslToolTip& self=GetInstance();

    if (self.m_frame && self.m_top && !top)
        return;

    ResetTip();

    self.m_top=top;
    self.m_parent=window;

    if (top)
        self.CreateFrame();
    else
        self.m_timer.Start(delay, wxTIMER_ONE_SHOT);
}

void CslToolTip::ResetTip()
{
    CslToolTip& self=GetInstance();

    if (!self.m_parent)
        return;

    self.m_parent=NULL;

    if (self.m_frame)
    {
        self.m_frame->Destroy();
        self.m_frame=NULL;
    }

    if (self.m_timer.IsRunning())
        self.m_timer.Stop();
}
