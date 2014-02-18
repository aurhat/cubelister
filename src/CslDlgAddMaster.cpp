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
#include "CslEngine.h"
#include "CslApp.h"
#include "CslDlgAddMaster.h"


BEGIN_EVENT_TABLE(CslDlgAddMaster,wxDialog)
    EVT_TEXT(wxID_ANY, CslDlgAddMaster::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY, CslDlgAddMaster::OnCommandEvent)
    EVT_CHOICE(wxID_ANY, CslDlgAddMaster::OnCommandEvent)
    EVT_BUTTON(wxID_ANY, CslDlgAddMaster::OnCommandEvent)
END_EVENT_TABLE()

enum
{
    CHOICE_CTRL_GAMETYPE    = wxID_HIGHEST + 1,
    CHOICE_CTRL_MASTERTYPE,
    TEXT_CTRL_ADDRESS,
    TEXT_CTRL_PATH
};


CslDlgAddMaster::CslDlgAddMaster(wxWindow* parent):
        wxDialog(parent, wxID_ANY, wxString(wxEmptyString)),
        m_modified(false), m_fourcc(NULL)
{
    // begin wxGlade: CslDlgAddMaster::CslDlgAddMaster
    sizer_master_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString m_choiceGameType_choices[] = {
        _("default")
    };
    m_choiceGameType = new wxChoice(this, CHOICE_CTRL_GAMETYPE, wxDefaultPosition, wxDefaultSize, 1, m_choiceGameType_choices, 0);
    const wxString m_choiceMasterType_choices[] = {
        _("default")
    };
    m_choiceMasterType = new wxChoice(this, CHOICE_CTRL_MASTERTYPE, wxDefaultPosition, wxDefaultSize, 1, m_choiceMasterType_choices, 0);
    m_tcAddress = new wxTextCtrl(this, TEXT_CTRL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_scPort = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    m_tcPath = new wxTextCtrl(this, TEXT_CTRL_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_bsDlg = CreateButtonSizer(wxOK|wxCANCEL);

    set_properties();
    do_layout();
    // end wxGlade
}

void CslDlgAddMaster::set_properties()
{
    // begin wxGlade: CslDlgAddMaster::set_properties
    SetTitle(_("CSL - Add new Master"));
    m_choiceGameType->SetSelection(0);
    m_choiceMasterType->SetSelection(0);
    m_tcAddress->SetMinSize(wxSize(180, -1));
    m_tcAddress->SetFocus();
    m_scPort->SetMinSize(wxSize(80, -1));
    // end wxGlade

    wxButton *buttonOK = (wxButton*)FindWindowById(wxID_OK, this);
    buttonOK->Enable(false);

    CSL_SET_WINDOW_ICON();
}

void CslDlgAddMaster::do_layout()
{
    // begin wxGlade: CslDlgAddMaster::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_master = new wxStaticBoxSizer(sizer_master_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_master = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_address = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_game = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticText* label_game = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_master->Add(label_game, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_game->Add(m_choiceGameType, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_master = new wxStaticText(this, wxID_ANY, _("Master type:"));
    grid_sizer_game->Add(label_master, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_game->Add(m_choiceMasterType, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_game->AddGrowableRow(0);
    grid_sizer_game->AddGrowableCol(0);
    grid_sizer_game->AddGrowableCol(2);
    grid_sizer_master->Add(grid_sizer_game, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* label_address = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_master->Add(label_address, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(m_tcAddress, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_port = new wxStaticText(this, wxID_ANY, _("Port:"));
    grid_sizer_address->Add(label_port, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(m_scPort, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->AddGrowableRow(0);
    grid_sizer_address->AddGrowableCol(0);
    grid_sizer_address->AddGrowableCol(2);
    grid_sizer_master->Add(grid_sizer_address, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText* label_path = new wxStaticText(this, wxID_ANY, _("Path:"));
    grid_sizer_master->Add(label_path, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_master->Add(m_tcPath, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_master->AddGrowableRow(0);
    grid_sizer_master->AddGrowableRow(1);
    grid_sizer_master->AddGrowableRow(2);
    grid_sizer_master->AddGrowableCol(1);
    sizer_master->Add(grid_sizer_master, 1, wxTOP|wxEXPAND, 2);
    grid_sizer_main->Add(sizer_master, 1, wxALL|wxEXPAND, 4);
    grid_sizer_main->Add(m_bsDlg, 1, wxALL|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    CSL_CENTRE_DIALOG();
}

void CslDlgAddMaster::InitDlg(wxUint32 *fourcc)
{
    m_fourcc = fourcc;

    m_choiceGameType->Clear();
    m_choiceMasterType->Clear();

    wxInt32 selected = -1;
    CslGame *game;
    CslArrayCslGame& games = ::wxGetApp().GetCslEngine()->GetGames();

    loopv(games)
    {
        game = games[i];
        wxUint32 fcc = game->GetFourCC();
        m_choiceGameType->Append(game->GetName(), (void*)(wxUIntPtr)fcc);

        if (*fourcc==fcc)
            selected = i;
    }

    m_choiceMasterType->Append(wxT("HTTP"));
    m_choiceMasterType->Append(wxT("TCP"));

    m_choiceGameType->SetSelection(selected<0 ? 0 : selected);

    SetGameDefaultValues();
}

bool CslDlgAddMaster::IsValid()
{
    return !m_tcAddress->GetValue().IsEmpty();
}

void CslDlgAddMaster::SetGameDefaultValues()
{
    CslGame *game = GetSelectedGame();

    if (game)
    {
        wxURI uri(game->GetDefaultMasterURI());
        bool http = uri.GetScheme().CmpNoCase(wxT("http"))==0;
        wxUint16 port = uri.HasPort() ? ::wxAtoi(uri.GetPort()) : 80;

        m_choiceMasterType->SetSelection(http ? 0 : 1);
        m_scPort->SetValue(port);
    }
    else
    {
        m_choiceMasterType->SetSelection(0);
        m_scPort->SetValue(80);
    }
}

CslGame* CslDlgAddMaster::GetSelectedGame(wxUint32 *pos)
{
    CslArrayCslGame& games = ::wxGetApp().GetCslEngine()->GetGames();

    wxInt32 selection = m_choiceGameType->GetSelection();
    wxUint32 fourcc = (wxUint32)(wxUIntPtr)m_choiceGameType->GetClientData(selection);

    loopv(games)
    {
        CslGame *game = games[i];

        if (fourcc==game->GetFourCC())
        {
            if (pos)
                *pos = i;

            return game;
        }
    }

    return NULL;
}

void CslDlgAddMaster::OnCommandEvent(wxCommandEvent& event)
{
    wxButton *buttonOK = (wxButton*)FindWindowById(wxID_OK, this);

    switch (event.GetId())
    {
        case CHOICE_CTRL_MASTERTYPE:
        {
            m_modified = true;
            buttonOK->Enable(IsValid());
            break;
        }

        case CHOICE_CTRL_GAMETYPE:
            if (!m_modified)
                SetGameDefaultValues();
        case TEXT_CTRL_ADDRESS:
        case TEXT_CTRL_PATH:
            if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED)
            {
                m_modified = true;
                buttonOK->Enable(IsValid());
                break;
            }
            else if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;

        case wxID_OK:
        {
            // check again for empty host, since setting a default button on wxMAC
            // and hitting enter reaches this position, also if the the button was disabled
            if (!IsValid())
                break;


            CslGame *game = GetSelectedGame(m_fourcc);

            if (!game)
            {
                EndModal(wxID_CANCEL);
                break;
            }

            wxURI uri(CslMaster::CreateURI(m_choiceMasterType->GetStringSelection(),
                                           m_tcAddress->GetValue(),
                                           m_scPort->GetValue(),
                                           m_tcPath->GetValue()));


            CslArrayCslMaster& masters = game->GetMasters();

            loopv(masters)
            {
                if (masters[i]->GetURI()==uri)
                {
                    wxMessageBox(_("Master already exists!"), _("Error"), wxICON_ERROR, this);
                    return;
                }
            }

            game->AddMaster(CslMaster::Create(uri));

            EndModal(wxID_OK);
            break;
        }

        case wxID_CANCEL:
            EndModal(wxID_CANCEL);
            break;
    }
}
