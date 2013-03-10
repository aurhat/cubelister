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
#include "CslDlgAddMaster.h"


BEGIN_EVENT_TABLE(CslDlgAddMaster,wxDialog)
    EVT_TEXT(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_BUTTON(wxID_ADD,CslDlgAddMaster::OnCommandEvent)
END_EVENT_TABLE()

enum
{
    CHOICE_CTRL_GAMETYPE    = wxID_HIGHEST + 1,
    CHOICE_CTRL_MASTERTYPE,
    TEXT_CTRL_ADDRESS,
    TEXT_CTRL_PATH,
};


CslDlgAddMaster::CslDlgAddMaster(wxWindow* parent, wxInt32 id,
                                 const wxString& title,const wxPoint& pos,
                                 const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style),
        m_fourcc(NULL)
{
    // begin wxGlade: CslDlgAddMaster::CslDlgAddMaster
    sizer_address_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString choice_gametype_choices[] = {
        _("default")
    };
    choice_gametype = new wxChoice(this, CHOICE_CTRL_GAMETYPE, wxDefaultPosition, wxDefaultSize, 1, choice_gametype_choices, 0);
    const wxString choice_mastertype_choices[] = {
        _("default")
    };
    choice_mastertype = new wxChoice(this, CHOICE_CTRL_MASTERTYPE, wxDefaultPosition, wxDefaultSize, 1, choice_mastertype_choices, 0);
    text_ctrl_address = new wxTextCtrl(this, TEXT_CTRL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    spin_ctrl_port = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    text_ctrl_path = new wxTextCtrl(this, TEXT_CTRL_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    button_add = new wxButton(this, wxID_ADD, _("Add"));
    button_cancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

    set_properties();
    do_layout();
    // end wxGlade
}

void CslDlgAddMaster::set_properties()
{
    // begin wxGlade: CslDlgAddMaster::set_properties
    SetTitle(_("CSL - Add new Master"));
    choice_gametype->SetSelection(0);
    choice_mastertype->SetSelection(0);
    text_ctrl_address->SetMinSize(wxSize(180, -1));
    text_ctrl_address->SetFocus();
    spin_ctrl_port->SetMinSize(wxSize(80, -1));
    button_add->Enable(false);
    button_add->SetDefault();
    // end wxGlade

    CSL_SET_WINDOW_ICON();
}

void CslDlgAddMaster::do_layout()
{
    // begin wxGlade: CslDlgAddMaster::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_address = new wxStaticBoxSizer(sizer_address_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_address = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_address_copy = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_choice = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticText* label_game_static_copy = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_address->Add(label_game_static_copy, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_choice->Add(choice_gametype, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_master_static = new wxStaticText(this, wxID_ANY, _("Master type:"));
    grid_sizer_choice->Add(label_master_static, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_choice->Add(choice_mastertype, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_choice->AddGrowableCol(0);
    grid_sizer_choice->AddGrowableCol(2);
    grid_sizer_address->Add(grid_sizer_choice, 1, wxEXPAND, 0);
    wxStaticText* label_address_static = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_address->Add(label_address_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_copy->Add(text_ctrl_address, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_port_static = new wxStaticText(this, wxID_ANY, _("Port:"));
    grid_sizer_address_copy->Add(label_port_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_copy->Add(spin_ctrl_port, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(grid_sizer_address_copy, 1, wxEXPAND, 0);
    wxStaticText* label_path_static = new wxStaticText(this, wxID_ANY, _("Path:"));
    grid_sizer_address->Add(label_path_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(text_ctrl_path, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->AddGrowableCol(1);
    sizer_address->Add(grid_sizer_address, 1, wxTOP|wxEXPAND, 2);
    grid_sizer_main->Add(sizer_address, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(1, 1, 0, 0, 0);
    grid_sizer_button->Add(button_add, 0, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(button_cancel, 0, wxALL|wxEXPAND, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    CSL_CENTER_DIALOG();
}

void CslDlgAddMaster::InitDlg(wxUint32 *fourcc)
{
    wxInt32 id=-1;

    m_fourcc=fourcc;

    choice_gametype->Clear();
    choice_mastertype->Clear();

    CslArrayCslGame& games=::wxGetApp().GetCslEngine()->GetGames();
    loopv(games)
    {
        wxUint32 fcc=games[i]->GetFourCC();
        choice_gametype->Append(games[i]->GetName(), (void*)(wxUIntPtr)fcc);

        if (*fourcc==fcc)
            id=i;
    }

    choice_mastertype->Append(wxT("HTTP"),(void*)CslMasterConnection::CONNECTION_HTTP);
    choice_mastertype->Append(wxT("TCP"),(void*)CslMasterConnection::CONNECTION_TCP);

    choice_gametype->SetSelection(id<0 ? 0 : id);
    choice_mastertype->SetSelection(0);
    spin_ctrl_port->SetValue(CSL_DEFAULT_MASTER_WEB_PORT);
}

bool CslDlgAddMaster::IsValid()
{
    return !(text_ctrl_address->GetValue().IsEmpty() ||
             (text_ctrl_path->GetValue().IsEmpty() && text_ctrl_path->IsEnabled()));
}

void CslDlgAddMaster::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case CHOICE_CTRL_MASTERTYPE:
        {
            bool path=(wxInt32)(long)choice_mastertype->GetClientData(choice_mastertype->GetSelection()) ==
                      CslMasterConnection::CONNECTION_HTTP;
            text_ctrl_path->Enable(path);
            button_add->Enable(IsValid());
            break;
        }

        case CHOICE_CTRL_GAMETYPE:
        case TEXT_CTRL_ADDRESS:
        case TEXT_CTRL_PATH:
            if (event.GetEventType()==wxEVT_COMMAND_TEXT_UPDATED)
            {
                button_add->Enable(IsValid());
                break;
            }
            else if (event.GetEventType()!=wxEVT_COMMAND_TEXT_ENTER)
                break;

        case wxID_ADD:
        {
            // check again for empty host, since setting a default button on wxMAC
            // and hitting enter reaches this position, also if the the button was disabled
            if (!IsValid())
                break;

            wxUint32 fourcc=(wxUint32)(long)choice_gametype->GetClientData(choice_gametype->GetSelection());
            wxInt32 type=(wxInt32)(long)choice_mastertype->GetClientData(choice_mastertype->GetSelection());
            CslGame *game=NULL;
            CslArrayCslGame& games=::wxGetApp().GetCslEngine()->GetGames();

            loopv(games)
            {
                if (games[i]->GetFourCC()==fourcc)
                {
                    game=games[i];
                    *m_fourcc=i;
                    break;
                }
            }
            if (!game)
            {
                EndModal(wxID_CANCEL);
                break;
            }

            CslMasterConnection connection;
            wxString addr=text_ctrl_address->GetValue();
            wxString path=text_ctrl_path->GetValue();
            wxUint16 port=spin_ctrl_port->GetValue();

            if (type==CslMasterConnection::CONNECTION_HTTP)
            {
                if (!path.StartsWith(wxT("/")))
                    path=wxT("/")+path;
                connection=CslMasterConnection(addr,path,port);
            }
            else
                connection=CslMasterConnection(addr,port);

            CslArrayCslMaster& masters=game->GetMasters();
            loopv(masters)
            {
                if (masters[i]->GetConnection()==connection)
                {
                    wxMessageBox(_("Master already exists!"),_("Error"),wxICON_ERROR,this);
                    return;
                }
            }

            CslMaster *master = CslMaster::Create(connection);
            game->AddMaster(master);
            EndModal(wxID_OK);
            break;
        }
        default:
            break;
    }
}
