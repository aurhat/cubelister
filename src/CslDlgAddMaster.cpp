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

#include "CslDlgAddMaster.h"

BEGIN_EVENT_TABLE(CslDlgAddMaster,wxDialog)
    EVT_TEXT(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_TEXT_ENTER(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_BUTTON(wxID_ADD,CslDlgAddMaster::OnCommandEvent)
    EVT_RADIOBUTTON(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
END_EVENT_TABLE()

enum
{
    CHOICE_CTRL_GAMETYPE    = wxID_HIGHEST + 1,
    CHOICE_CTRL_MASTERTYPE,
    TEXT_CTRL_ADDRESS,
    TEXT_CTRL_PATH,
    RADIO_CTRL_CUSTOM,
    RADIO_CTRL_DEFAULT
};


CslDlgAddMaster::CslDlgAddMaster(wxWindow* parent,const wxInt32 id,
                                 const wxString& title,const wxPoint& pos,
                                 const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style),
        m_engine(NULL),m_gameID(NULL)
{
    // begin wxGlade: CslDlgAddMaster::CslDlgAddMaster
    sizer_address_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString choice_gametype_choices[] =
    {
        _("default")
    };
    choice_gametype = new wxChoice(this, CHOICE_CTRL_GAMETYPE, wxDefaultPosition, wxDefaultSize, 1, choice_gametype_choices, 0);
    const wxString choice_mastertype_choices[] =
    {
        _("default")
    };
    choice_mastertype = new wxChoice(this, CHOICE_CTRL_MASTERTYPE, wxDefaultPosition, wxDefaultSize, 1, choice_mastertype_choices, 0);
    text_ctrl_address = new wxTextCtrl(this, TEXT_CTRL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    spin_ctrl_port = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535);
    text_ctrl_path = new wxTextCtrl(this, TEXT_CTRL_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    radio_btn_custom = new wxRadioButton(this, RADIO_CTRL_CUSTOM, _("C&ustom"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    radio_btn_default = new wxRadioButton(this, RADIO_CTRL_DEFAULT, _("&Default master"));
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
    text_ctrl_address->SetMinSize(wxSize(220, -1));
    text_ctrl_address->SetFocus();
    button_add->Enable(false);
    button_add->SetDefault();
    // end wxGlade
}

void CslDlgAddMaster::do_layout()
{
    // begin wxGlade: CslDlgAddMaster::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_address = new wxStaticBoxSizer(sizer_address_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_input = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_radio = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_address_top = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_address = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_choice = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticText* label_game_static = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_address_top->Add(label_game_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_choice->Add(choice_gametype, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_master_static = new wxStaticText(this, wxID_ANY, _("Master type:"));
    grid_sizer_choice->Add(label_master_static, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_choice->Add(choice_mastertype, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_choice->AddGrowableCol(0);
    grid_sizer_choice->AddGrowableCol(2);
    grid_sizer_address_top->Add(grid_sizer_choice, 1, wxEXPAND, 0);
    wxStaticText* label_address_static = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_address_top->Add(label_address_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(text_ctrl_address, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_port_static = new wxStaticText(this, wxID_ANY, _("Port:"));
    grid_sizer_address->Add(label_port_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(spin_ctrl_port, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->Add(grid_sizer_address, 1, wxEXPAND, 0);
    wxStaticText* label_path_static = new wxStaticText(this, wxID_ANY, _("Path:"));
    grid_sizer_address_top->Add(label_path_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->Add(text_ctrl_path, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->AddGrowableCol(1);
    grid_sizer_input->Add(grid_sizer_address_top, 1, wxTOP|wxEXPAND, 2);
    grid_sizer_radio->Add(radio_btn_custom, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_radio->Add(radio_btn_default, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_radio->AddGrowableCol(0);
    grid_sizer_radio->AddGrowableCol(1);
    grid_sizer_input->Add(grid_sizer_radio, 1, wxEXPAND, 0);
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

void CslDlgAddMaster::InitDlg(CslEngine *engine,wxInt32 *gameID)
{
    wxInt32 id=-1;

    m_engine=engine;
    m_gameID=gameID;

    choice_gametype->Clear();
    choice_mastertype->Clear();

    vector<CslGame*>& games=engine->GetGames();
    loopv(games)
    {
        choice_gametype->Append(games[i]->GetName(),(void*)games[i]->GetId());
        if (games[i]->GetId()==*gameID)
            id=i;
    }

    choice_mastertype->Append(wxT("HTTP"),(void*)CslMasterConnection::CONNECTION_HTTP);
    choice_mastertype->Append(wxT("Other"),(void*)CslMasterConnection::CONNECTION_OTHER);

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
    if (!m_engine)
        return;

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
        {
            // dont break if default is set
            if (!radio_btn_default->GetValue())
                break;
        }

        case RADIO_CTRL_DEFAULT:
        {
            wxString addr,path;
            wxUint32 c;
            wxInt32 gameID=(wxInt32)(long)choice_gametype->GetClientData(choice_gametype->GetSelection());

            vector<CslGame*>& games=m_engine->GetGames();
            loopv(games)
            {
                if (games[i]->GetId()!=gameID)
                    continue;

                CslMasterConnection& connection=games[i]->GetDefaultMasterConnection();

                for (c=0;c<choice_mastertype->GetCount();c++)
                {
                    if ((wxInt32)(long)choice_mastertype->GetClientData(c)==connection.GetType())
                    {
                        choice_mastertype->SetSelection(c);
                        break;
                    }
                }
                choice_mastertype->Enable(false);
                text_ctrl_address->Enable(false);
                spin_ctrl_port->Enable(false);
                text_ctrl_path->Enable(false);

                text_ctrl_address->SetValue(connection.GetAddress());
                spin_ctrl_port->SetValue(connection.GetPort());
                text_ctrl_path->SetValue(connection.GetPath());
                break;
            }
            //button_add->Enable();
            break;
        }

        case RADIO_CTRL_CUSTOM:
        {
            bool path=(wxInt32)(long)choice_mastertype->GetClientData(choice_mastertype->GetSelection()) ==
                      CslMasterConnection::CONNECTION_HTTP;
            choice_mastertype->Enable();
            text_ctrl_address->Enable();
            spin_ctrl_port->Enable();
            text_ctrl_path->Enable(path);
            break;
        }

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

            wxInt32 gameID=(wxInt32)(long)choice_gametype->GetClientData(choice_gametype->GetSelection());
            wxInt32 type=(wxInt32)(long)choice_mastertype->GetClientData(choice_mastertype->GetSelection());
            CslGame *game=NULL;
            vector<CslGame*>& games=m_engine->GetGames();

            loopv(games)
            {
                if (games[i]->GetId()==gameID)
                {
                    game=games[i];
                    *m_gameID=i;
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
                connection=CslMasterConnection(addr,path);
            }
            else
                connection=CslMasterConnection(addr,port);

            vector<CslMaster*>& masters=game->GetMasters();
            loopv(masters)
            {
                if (masters[i]->GetConnection()==connection)
                {
                    wxMessageBox(_("Master already exists!"),_("Error"),wxICON_ERROR,this);
                    return;
                }
            }

            CslMaster *master=new CslMaster(connection);
            game->AddMaster(master);
            EndModal(wxID_OK);
            break;
        }
        default:
            break;
    }
}
