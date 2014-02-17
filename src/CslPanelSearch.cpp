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

#include "Csl.h"
#include "CslMenu.h"
#include "CslPanelSearch.h"

// begin wxGlade: ::extracode

// end wxGlade


BEGIN_EVENT_TABLE(CslPanelSearch, wxPanel)
EVT_SHOW(CslPanelSearch::OnShow)
EVT_TEXT(wxID_ANY, CslPanelSearch::OnCommandEvent)
EVT_BUTTON(wxID_ANY, CslPanelSearch::OnCommandEvent)
EVT_RADIOBUTTON(wxID_ANY, CslPanelSearch::OnCommandEvent)
END_EVENT_TABLE()


CslPanelSearch::CslPanelSearch(wxWindow* parent, int id,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style):
    wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL)
{
    // begin wxGlade: CslPanelSearch::CslPanelSearch
    m_search_ctrl = new wxSearchCtrl(this, SEARCH_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_radio_server = new wxRadioButton(this, SEARCH_RADIO_SERVER, _("Serve&rs"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_radio_player = new wxRadioButton(this, SEARCH_RADIO_PLAYER, _("&Players"));
    m_gauge_progress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH);
    m_label_result_prefix = new wxStaticText(this, wxID_ANY, _("Result:"));
    m_label_result = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_button_search = new wxButton(this, SEARCH_BUTTON_SEARCH, _("&Search"));

    set_properties();
    do_layout();
    // end wxGlade

    wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, SEARCH_RADIO_SERVER);
    ProcessEvent(evt);

    // depending on platform, key events from childs never reach
    // the main frame using static event tables - connect them here
    ConnectEventRecursive(wxID_ANY, this, this, wxEVT_KEY_DOWN, wxKeyEventHandler(CslPanelSearch::OnKeypress));
}


void CslPanelSearch::set_properties()
{
    // begin wxGlade: CslPanelSearch::set_properties
    m_search_ctrl->SetMinSize(wxSize(120, -1));
    m_gauge_progress->SetMinSize(wxSize(-1, 16));
    // end wxGlade

    wxFont font = m_label_result_prefix->GetFont();
    font.SetWeight(wxBOLD);
    m_label_result_prefix->SetFont(font);
}


void CslPanelSearch::do_layout()
{
    // begin wxGlade: CslPanelSearch::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(3, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_result = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_radio_progress = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_progress = new wxFlexGridSizer(3, 1, 0, 0);
    grid_sizer_main->Add(m_search_ctrl, 1, wxALL|wxEXPAND, 4);
    grid_sizer_radio_progress->Add(m_radio_server, 0, wxALL, 4);
    grid_sizer_radio_progress->Add(m_radio_player, 0, wxALL, 4);
    grid_sizer_progress->Add(1, 0, 0, 0, 0);
    grid_sizer_progress->Add(m_gauge_progress, 0, wxALL|wxEXPAND, 4);
    grid_sizer_progress->Add(1, 0, 0, 0, 0);
    grid_sizer_progress->AddGrowableRow(0);
    grid_sizer_progress->AddGrowableRow(2);
    grid_sizer_progress->AddGrowableCol(0);
    grid_sizer_radio_progress->Add(grid_sizer_progress, 1, wxEXPAND, 0);
    grid_sizer_radio_progress->AddGrowableCol(2);
    grid_sizer_main->Add(grid_sizer_radio_progress, 1, wxEXPAND, 0);
    grid_sizer_result->Add(m_label_result_prefix, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_result->Add(m_label_result, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_result->Add(m_button_search, 0, wxALL, 4);
    grid_sizer_result->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_result, 1, wxEXPAND, 0);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableCol(0);
    // end wxGlade

    m_sizer_result = grid_sizer_result;
}

void CslPanelSearch::SetResult(const wxString& text, const wxString& prefix)
{
    wxWindowUpdateLocker lock(this);

    if (!prefix.IsEmpty())
        m_label_result_prefix->SetLabel(prefix);

    m_label_result->SetLabel(text);

    m_sizer_result->Layout();
}

void CslPanelSearch::OnShow(wxShowEvent& event)
{
    if (event.IsShown())
        m_search_ctrl->SetFocus();

    event.Skip();
}

void CslPanelSearch::OnKeypress(wxKeyEvent& event)
{
    wxWindow *wnd = wxDynamicCast(event.GetEventObject(), wxWindow);

    if (event.GetKeyCode()==WXK_ESCAPE &&
        WindowHasChildWindow(m_search_ctrl, wnd))
    {
        m_search_ctrl->Clear();

        wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED, SEARCH_TEXT);
        AddPendingEvent(evt);
    }

    event.Skip();
}

void CslPanelSearch::OnCommandEvent(wxCommandEvent& event)
{
    wxInt32 id = event.GetId();

    switch (id)
    {
        case SEARCH_RADIO_SERVER:
        case SEARCH_RADIO_PLAYER:
        {
            bool empty = m_search_ctrl->GetValue().IsEmpty();

            m_button_search->Enable(id==SEARCH_RADIO_PLAYER && !empty);
            m_gauge_progress->Enable(id==SEARCH_RADIO_PLAYER && !empty);

            SetResult(_("none"));

            m_search_ctrl->SetFocus();
            SetSearchCtrlError(false);
            break;
        }

        case SEARCH_TEXT:
        {
            bool empty = m_search_ctrl->GetValue().IsEmpty();

            m_button_search->Enable(!empty && IsPlayerSearch());
            m_gauge_progress->Enable(!empty && IsPlayerSearch());
            SetSearchCtrlError(false);

            if (empty && IsServerSearch())
                m_label_result->SetLabel(_("none"));
            break;
        }

        case SEARCH_BUTTON_SEARCH:
            break;

        default:
            return;
    }

    GetParent()->GetEventHandler()->ProcessEvent(event);
}
