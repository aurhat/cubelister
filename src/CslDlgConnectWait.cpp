/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#include "CslDlgConnectWait.h"
#include "CslSettings.h"

BEGIN_EVENT_TABLE(CslDlgConnectWait,wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgConnectWait::OnButton)
    EVT_TEXT_ENTER(wxID_ANY,CslDlgConnectWait::OnButton)
END_EVENT_TABLE()

CslDlgConnectWait::CslDlgConnectWait(wxWindow* parent,wxInt32 *time,int id,const wxString& title,
                                     const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,style),m_time(time)
{
    // begin wxGlade: CslDlgConnectWait::CslDlgConnectWait
    spin_ctrl_time = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, 0, 100);
    checkbox_default = new wxCheckBox(this, wxID_ANY, _("Set as default waiting time."));
    button_ok = new wxButton(this, wxID_OK, _("&Wait"));
    button_connect = new wxButton(this, wxID_ANY, _("C&onnect"));
    button_cancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgConnectWait::set_properties()
{
    // begin wxGlade: CslDlgConnectWait::set_properties
    SetTitle(_("CSL - Server full"));
    button_ok->SetFocus();
    button_ok->SetDefault();
    // end wxGlade

    spin_ctrl_time->SetRange(CSL_WAIT_SERVER_FULL_MIN,CSL_WAIT_SERVER_FULL_MAX);
    spin_ctrl_time->SetValue(*m_time);
}


void CslDlgConnectWait::do_layout()
{
    // begin wxGlade: CslDlgConnectWait::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_top = new wxFlexGridSizer(3, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_spin = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticText* label_static = new wxStaticText(this, wxID_ANY, _("The server is full at the moment. Wait for a free slot?"));
    grid_sizer_top->Add(label_static, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    wxStaticText* label_spin_static = new wxStaticText(this, wxID_ANY, _("Time to wait (sec):"));
    grid_sizer_spin->Add(label_spin_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_spin->Add(spin_ctrl_time, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_top->Add(grid_sizer_spin, 1, wxEXPAND, 0);
    grid_sizer_top->Add(checkbox_default, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(grid_sizer_top, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(button_ok, 0, wxALL, 4);
    grid_sizer_button->Add(button_connect, 0, wxALL, 4);
    grid_sizer_button->Add(button_cancel, 0, wxALL, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    Layout();
    // end wxGlade

    CentreOnParent();
}

void CslDlgConnectWait::OnButton(wxCommandEvent& event)
{
    wxInt32 id=event.GetId();

    if (id==wxID_OK)
    {
        *m_time=spin_ctrl_time->GetValue();
        if (checkbox_default->IsChecked())
            g_cslSettings->waitServerFull=*m_time;
    }

    EndModal(id);
}
