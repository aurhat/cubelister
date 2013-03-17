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
#include "CslToolTip.h"

DEFINE_EVENT_TYPE(wxCSL_EVT_TOOLTIP)

IMPLEMENT_DYNAMIC_CLASS(CslToolTipEvent, wxEvent)

BEGIN_EVENT_TABLE(CslToolTip, wxEvtHandler)
    EVT_TIMER(wxID_ANY, CslToolTip::OnTimer)
END_EVENT_TABLE()


CslToolTip::CslToolTip() :
        m_top(false), m_processMouseButton(false), m_frame(NULL), m_parent(NULL)
{
    m_timer.SetOwner(this);
}

CslToolTip::~CslToolTip()
{
    Reset();
}

void CslToolTip::Init(wxWindow *window, wxInt32 delay, bool top)
{
    CslToolTip& self=GetInstance();

    if (self.m_frame && self.m_top && !top)
        return;

    Reset();

    self.m_top = top;
    self.m_parent = window;

    if (top)
        self.CreateFrame();
    else
        self.m_timer.Start(delay, wxTIMER_ONE_SHOT);
}

void CslToolTip::Reset()
{
    CslToolTip& self=GetInstance();

    if (!self.m_parent)
        return;

    self.m_parent = NULL;
    self.m_processMouseButton = false;

    if (self.m_frame)
    {
        self.m_frame->Destroy();
        self.m_frame = NULL;
    }

    if (self.m_timer.IsRunning())
        self.m_timer.Stop();
}

CslToolTip& CslToolTip::GetInstance()
{
    static CslToolTip tip;

    return tip;
}

void CslToolTip::CreateFrame()
{
    wxFrame *frame = new wxFrame(m_parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                 wxNO_BORDER|wxFRAME_TOOL_WINDOW|(m_top ? wxSTAY_ON_TOP : wxFRAME_FLOAT_ON_PARENT));

    CslToolTipEvent event(frame, ::wxGetMousePosition());

    if (!m_parent->GetEventHandler()->ProcessEvent(event))
    {
        delete frame;
        return;
    }

    bool custom = false;

    if (event.Title.IsEmpty() && event.Text.IsEmpty())
    {
        if (!event.UserWindow)
        {
            delete frame;
            return;
        }

        custom = true;
    }

    m_frame = frame;

    wxStaticText *title = NULL;
    wxStaticText *left = NULL;
    wxStaticText *right = NULL;
    wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);

    if (!event.Title.IsEmpty())
        title = new wxStaticText(frame, wxID_ANY, wxEmptyString);

    if (!event.Text.IsEmpty())
    {
        left = new wxStaticText(frame, wxID_ANY, wxEmptyString);
        right = new wxStaticText(frame, wxID_ANY, wxEmptyString);
    }

    if (!custom)
    {
        const wxColour bg = CSL_SYSCOLOUR(wxSYS_COLOUR_INFOBK);
        const wxColour fg = CSL_SYSCOLOUR(wxSYS_COLOUR_INFOTEXT);

        wxFont bold = title ? title->GetFont() : left->GetFont();
        bold.SetWeight(wxFONTWEIGHT_BOLD);

        if (title)
        {
            box->Add(title, 0, wxTOP|wxLEFT|wxRIGHT|wxCENTER|(left ? 0 : wxBOTTOM), left ? 6 : 4);
            title->SetForegroundColour(fg);
            title->SetBackgroundColour(bg);

            if (left)
                title->SetFont(bold);

            event.Title.Replace(wxT("&"), wxT("&&"));
            title->SetLabel(event.Title);
        }
        if (left)
        {
            wxFlexGridSizer *grid = new wxFlexGridSizer(1, 2, 0, 0);
            box->Add(grid, 1, wxLEFT|wxRIGHT|wxCENTER, 4);
            long style = wxEXPAND|wxBOTTOM|(title ? 0 : wxTOP);
            grid->Add(left, 0, style|wxLEFT, 6);
            grid->Add(right, 1, style|wxRIGHT, 6);
            grid->AddGrowableCol(0);
            grid->AddGrowableCol(1);
            left->SetForegroundColour(fg);
            right->SetForegroundColour(fg);
            left->SetBackgroundColour(bg);
            right->SetBackgroundColour(bg);

            wxString l, r;

            loopv(event.Text)
            {
                event.Text[i].Replace(wxT("&"), wxT("&&"));

                if (i%2==0)
                    l << wxT("\n") << event.Text[i];
                else
                    r << wxT("\n") << event.Text[i];
            }

            left->SetLabel(l);
            right->SetLabel(r);

            {
                wxClientDC dc(frame);
                wxInt32 gap = dc.GetTextExtent(wxT(" ")).x;
                grid->SetHGap(m_top ? gap : gap*4);
            }

        }
    }

    if (event.UserWindow)
        box->Add(event.UserWindow, 0, wxCENTRE|wxBOTTOM|wxLEFT|wxRIGHT, custom ? 0 : 6);

#ifdef __WXMAC__
    if (left)
    {
        wxClientDC dc(left);
        const wxSize size = dc.GetMultiLineTextExtent(l);
        left->SetClientSize(size);
    }
    if (right)
    {
        wxClientDC dc(right);
        const wxSize size = dc.GetMultiLineTextExtent(r);
        right->SetClientSize(size);
    }
#endif

    frame->SetSizer(box);
    box->Fit(frame);

    ConnectEventRecursive(wxID_ANY, frame, this, wxEVT_LEFT_UP,
                          wxMouseEventHandler(CslToolTip::OnMouseButton));
    ConnectEventRecursive(wxID_ANY, frame, this, wxEVT_RIGHT_UP,
                          wxMouseEventHandler(CslToolTip::OnMouseButton));
    ConnectEventRecursive(wxID_ANY, frame, this, wxEVT_MIDDLE_UP,
                          wxMouseEventHandler(CslToolTip::OnMouseButton));
    ConnectEventRecursive(wxID_ANY, frame, this, wxEVT_MOUSEWHEEL,
                          wxMouseEventHandler(CslToolTip::OnMouseButton));

    if (!custom)
    {
#ifdef __WXMSW__
        frame->Connect(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(CslToolTip::OnEraseBackground), NULL, this);
#else
        frame->Connect(wxEVT_PAINT, wxPaintEventHandler(CslToolTip::OnPaint), NULL, this);
#endif
    }
    frame->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(CslToolTip::OnMouseLeave), NULL, this);

#ifndef __WXGTK__
    const wxRect& client = frame->GetRect();
    const wxRect& screen = ::wxGetClientDisplayRect();

    // place single line tips above mouse and
    // make sure the tip is fully shown on the screen
    if (title && !left)
        event.Pos.y = max(event.Pos.y-client.height, 0);
    else
    {
        if (event.Pos.y+client.height>screen.height)
            event.Pos.y -= (event.Pos.y+client.height-screen.height);
    }

    if (event.Pos.x+client.width>screen.width)
        event.Pos.x -= (event.Pos.x+client.width-screen.width);
#endif

    frame->Move(event.Pos);
    frame->Show();

    if (m_top)
        m_timer.Start(10000, wxTIMER_ONE_SHOT);
}

#ifdef __WXMSW__
void CslToolTip::OnEraseBackground(wxEraseEvent& event)
{
    if (!m_frame)
        return;

    wxDC& dc = *event.GetDC();
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
    dc.SetPen(wxPen(CSL_SYSCOLOUR(wxSYS_COLOUR_INFOTEXT)));
#endif
    dc.SetBrush(wxBrush(CSL_SYSCOLOUR(wxSYS_COLOUR_INFOBK)));

    dc.Clear();
#ifdef __WXMAC__
    dc.DrawRectangle(0, 0, w, h);
#else
    dc.DrawRoundedRectangle(0, 0, w, h, 2);
#endif
}

void CslToolTip::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (!m_parent)
        return;

    if (m_top)
    {
        Reset();
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
            Reset();
    }

    event.Skip();
}

void CslToolTip::OnMouseButton(wxMouseEvent& event)
{
    // calling ProcessEvent() calls this function
    // again, since this handler was "connected"
    // skip the event and bail out
    if (m_processMouseButton)
    {
        event.Skip();
        return;
    }

    wxObject *object = event.GetEventObject();

    // check for "delayed" event
    if (object==this)
    {
        Reset();
        return;
    }

    m_processMouseButton = true;

    // give controls in UserWindow the chance to handle
    // this event before the frame is getting destroyed
    if (object && object->IsKindOf(CLASSINFO(wxEvtHandler)))
        ((wxEvtHandler*)object)->ProcessEvent(event);

    m_processMouseButton = false;

    // "delay" the event by posting another mouse event to the queue
    // so controls which post events in UserWindow have a chance
    // to process these event before the frame is getting destroyed
    wxMouseEvent evt(event.GetEventType());
    evt.SetEventObject(this);
    ::wxPostEvent(m_frame, evt);
}
