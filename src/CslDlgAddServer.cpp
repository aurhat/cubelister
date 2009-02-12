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

#include "CslDlgAddServer.h"
#include "CslApp.h"


enum
{
    CHOICE_CTRL_GAMETYPE = wxID_HIGHEST + 1,
    TEXT_CTRL_ADDRESS,
    SPIN_CTRL_GAMEPORT,
    SPIN_CTRL_INFOPORT
};


BEGIN_EVENT_TABLE(CslDlgAddServer,wxDialog)
    EVT_TEXT(wxID_ANY,CslDlgAddServer::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslDlgAddServer::OnCommandEvent)
    EVT_BUTTON(wxID_ADD,CslDlgAddServer::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY,CslDlgAddServer::OnCommandEvent)
    EVT_SPINCTRL(wxID_ANY,CslDlgAddServer::OnSpinCtrl)
END_EVENT_TABLE()


CslDlgAddServer::CslDlgAddServer(wxWindow* parent,int id,const wxString& title,
                                 const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style),
        m_info(NULL)
{
    // begin wxGlade: CslDlgAddServer::CslDlgAddServer
    sizer_address_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString choice_gametype_choices[] =
    {
        _("default")
    };
    choice_gametype = new wxChoice(this, CHOICE_CTRL_GAMETYPE, wxDefaultPosition, wxDefaultSize, 1, choice_gametype_choices, 0);
    text_ctrl_address = new wxTextCtrl(this, TEXT_CTRL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    spin_ctrl_gameport = new wxSpinCtrl(this, SPIN_CTRL_GAMEPORT, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    spin_ctrl_infoport = new wxSpinCtrl(this, SPIN_CTRL_INFOPORT, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    button_add = new wxButton(this, wxID_ADD, _("Add"));
    button_cancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgAddServer::set_properties()
{
    // begin wxGlade: CslDlgAddServer::set_properties
    SetTitle(_("CSL - Add new server"));
    choice_gametype->SetSelection(0);
    text_ctrl_address->SetFocus();
    spin_ctrl_gameport->SetMinSize(wxSize(80, -1));
    spin_ctrl_infoport->SetMinSize(wxSize(80, -1));
    button_add->Enable(false);
    button_add->SetDefault();
    // end wxGlade
}


void CslDlgAddServer::do_layout()
{
    // begin wxGlade: CslDlgAddServer::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_address = new wxStaticBoxSizer(sizer_address_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_input = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_port = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticText* label_game_static = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_input->Add(label_game_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_input->Add(choice_gametype, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_address_static = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_input->Add(label_address_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_input->Add(text_ctrl_address, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_gameport_static = new wxStaticText(this, wxID_ANY, _("Port:"));
    grid_sizer_input->Add(label_gameport_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_port->Add(spin_ctrl_gameport, 0, wxALL, 4);
    wxStaticText* label_infoport_static = new wxStaticText(this, wxID_ANY, _("Info port:"));
    grid_sizer_port->Add(label_infoport_static, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_port->Add(spin_ctrl_infoport, 0, wxALL, 4);
    grid_sizer_input->Add(grid_sizer_port, 1, 0, 0);
    grid_sizer_input->AddGrowableRow(1);
    grid_sizer_input->AddGrowableCol(0);
    sizer_address->Add(grid_sizer_input, 1, wxEXPAND, 0);
    grid_sizer_main->Add(sizer_address, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(1, 1, 0, 0, 0);
    grid_sizer_button->Add(button_add, 0, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(button_cancel, 0, wxALL|wxEXPAND, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    Layout();
    // end wxGlade

    CentreOnParent();
}

void CslDlgAddServer::InitDlg(CslServerInfo *info)
{
    m_info=info;

    choice_gametype->Clear();
    vector<CslGame*>& games=::wxGetApp().GetCslEngine()->GetGames();
    loopv(games) choice_gametype->Append(games[i]->GetName(),(void*)games[i]->GetId());
    choice_gametype->SetSelection(0);
    UpdatePort(SPIN_CTRL_GAMEPORT);
}

void CslDlgAddServer::UpdatePort(wxInt32 type)
{
    wxInt32 gameID=(wxInt32)(long)choice_gametype->GetClientData(choice_gametype->GetSelection());
    vector<CslGame*>& games=::wxGetApp().GetCslEngine()->GetGames();

    loopv(games)
    {
        if (games[i]->GetId()==gameID)
        {
            if (type==SPIN_CTRL_GAMEPORT)
                spin_ctrl_gameport->SetValue(games[i]->GetDefaultGamePort());
            spin_ctrl_infoport->SetValue(games[i]->GetInfoPort(spin_ctrl_gameport->GetValue()));
            break;
        }
    }
}

void CslDlgAddServer::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case CHOICE_CTRL_GAMETYPE:
            UpdatePort(SPIN_CTRL_GAMEPORT);
            break;

        case SPIN_CTRL_GAMEPORT:
            UpdatePort(SPIN_CTRL_INFOPORT);
            break;

        case TEXT_CTRL_ADDRESS:
            if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED)
            {
                button_add->Enable(!text_ctrl_address->GetValue().IsEmpty());
                break;
            }
            if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;
        case wxID_ADD:
        {
            wxString host=text_ctrl_address->GetValue();
            wxUint16 gameport=spin_ctrl_gameport->GetValue();
            wxUint16 infoport=spin_ctrl_infoport->GetValue();

            // check again for empty host - since setting a default button on wxMAC
            // and hitting enter reaches this position, also if the the button was disabled
            if (host.IsEmpty())
                return;

            wxInt32 gameID=(wxInt32)(long)choice_gametype->GetClientData(choice_gametype->GetSelection());

            vector<CslGame*>& games=::wxGetApp().GetCslEngine()->GetGames();
            loopv(games)
            {
                if (games[i]->GetId()==gameID)
                {
                    m_info->Create(games[i],host,gameport,infoport);
                    if (games[i]->AddServer(m_info))
                    {
                        ::wxGetApp().GetCslEngine()->ResolveHost(m_info);
                        EndModal(wxID_OK);
                        return;
                    }
                    break;
                }
            }

            EndModal(wxID_CANCEL);
            break;
        }

        default:
            break;
    }
}

void CslDlgAddServer::OnSpinCtrl(wxSpinEvent& event)
{
    if (event.GetId()==SPIN_CTRL_GAMEPORT)
        UpdatePort(SPIN_CTRL_INFOPORT);
}
