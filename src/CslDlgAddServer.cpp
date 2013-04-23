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
#include "CslEngine.h"
#include "CslApp.h"
#include "CslDlgAddServer.h"


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


CslDlgAddServer::CslDlgAddServer(wxWindow* parent):
        wxDialog(parent, wxID_ANY, wxString(wxEmptyString)),
        m_info(NULL)
{
    // begin wxGlade: CslDlgAddServer::CslDlgAddServer
    sizer_address_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString m_choiceGameType_choices[] = {
        _("default")
    };
    m_choiceGameType = new wxChoice(this, CHOICE_CTRL_GAMETYPE, wxDefaultPosition, wxDefaultSize, 1, m_choiceGameType_choices, 0);
    m_tcAddress = new wxTextCtrl(this, TEXT_CTRL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_scGamePort = new wxSpinCtrl(this, SPIN_CTRL_GAMEPORT, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    m_scInfoPort = new wxSpinCtrl(this, SPIN_CTRL_INFOPORT, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    m_bsDlg = CreateButtonSizer(wxOK|wxCANCEL);

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgAddServer::set_properties()
{
    // begin wxGlade: CslDlgAddServer::set_properties
    SetTitle(_("CSL - Add new server"));
    m_choiceGameType->SetSelection(0);
    m_tcAddress->SetFocus();
    m_scGamePort->SetMinSize(wxSize(80, -1));
    m_scInfoPort->SetMinSize(wxSize(80, -1));
    // end wxGlade

    wxButton *buttonOK = (wxButton*)FindWindowById(wxID_OK, this);
    buttonOK->Enable(false);

    CSL_SET_WINDOW_ICON();
}


void CslDlgAddServer::do_layout()
{
    // begin wxGlade: CslDlgAddServer::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_address = new wxStaticBoxSizer(sizer_address_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_input = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_port = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticText* label_game = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_input->Add(label_game, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_input->Add(m_choiceGameType, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_address = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_input->Add(label_address, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_input->Add(m_tcAddress, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_gameport = new wxStaticText(this, wxID_ANY, _("Port:"));
    grid_sizer_input->Add(label_gameport, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_port->Add(m_scGamePort, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_infoport = new wxStaticText(this, wxID_ANY, _("Info port:"));
    grid_sizer_port->Add(label_infoport, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_port->Add(m_scInfoPort, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_port->AddGrowableRow(0);
    grid_sizer_port->AddGrowableCol(0);
    grid_sizer_port->AddGrowableCol(2);
    grid_sizer_input->Add(grid_sizer_port, 1, wxEXPAND, 0);
    grid_sizer_input->AddGrowableRow(0);
    grid_sizer_input->AddGrowableRow(1);
    grid_sizer_input->AddGrowableRow(2);
    grid_sizer_input->AddGrowableCol(1);
    sizer_address->Add(grid_sizer_input, 1, wxEXPAND, 0);
    grid_sizer_main->Add(sizer_address, 1, wxALL|wxEXPAND, 4);
    grid_sizer_main->Add(m_bsDlg, 1, wxALL|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    SetMinSize(GetClientSize());
    grid_sizer_main->SetSizeHints(this);

    CSL_CENTRE_DIALOG();
}

CslServerInfo* CslDlgAddServer::InitDlg(CslServerInfo *info)
{
    m_info = info;
    wxInt32 selected = -1;

    m_choiceGameType->Clear();

    CslArrayCslGame& games=::wxGetApp().GetCslEngine()->GetGames();

    loopv(games)
    {
        CslGame *game = games[i];

        if (selected<0 && info->GetGame().GetFourCC()==game->GetFourCC())
            selected = i;

        m_choiceGameType->Append(game->GetName(), (void*)(wxUIntPtr)game->GetFourCC());
    }

    if (!games.IsEmpty())
        m_choiceGameType->SetSelection(selected>-1 ? selected : 0);

    UpdatePort(SPIN_CTRL_GAMEPORT);

    return info;
}

void CslDlgAddServer::UpdatePort(wxInt32 type)
{
    wxUint32 fourcc=(wxUint32)(long)m_choiceGameType->GetClientData(m_choiceGameType->GetSelection());
    CslArrayCslGame& games=::wxGetApp().GetCslEngine()->GetGames();

    loopv(games)
    {
        CslGame *game = games[i];

        if (game->GetFourCC()==fourcc)
        {
            if (type==SPIN_CTRL_GAMEPORT)
                m_scGamePort->SetValue(game->GetDefaultGamePort());

             m_scInfoPort->SetValue(game->GetInfoPort(m_scGamePort->GetValue()));
             break;
        }
    }
}

void CslDlgAddServer::OnCommandEvent(wxCommandEvent& event)
{
    wxButton *buttonOK = (wxButton*)FindWindowById(wxID_OK, this);

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
                buttonOK->Enable(!m_tcAddress->GetValue().IsEmpty());
                break;
            }
            if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;
        case wxID_OK:
        {
            wxString host=m_tcAddress->GetValue();
            wxUint16 gameport=m_scGamePort->GetValue();
            wxUint16 infoport=m_scInfoPort->GetValue();

            // check again for empty host - since setting a default button on wxMAC
            // and hitting enter reaches this position, also if the the button was disabled
            if (host.IsEmpty())
                return;

            CslArrayCslGame& games=::wxGetApp().GetCslEngine()->GetGames();
            wxUint32 fourcc=(wxUint32)(long)m_choiceGameType->GetClientData(m_choiceGameType->GetSelection());

            loopv(games)
            {
                CslGame *game = games[i];

                if (game->GetFourCC()==fourcc)
                {
                    CslEngine *engine = ::wxGetApp().GetCslEngine();

                    m_info->Init(game, host, gameport, infoport);

                    if (engine->AddServer(game, m_info))
                    {
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
