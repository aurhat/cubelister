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
    EVT_TEXT(wxID_ANY,CslDlgAddMaster::OnText)
    EVT_TEXT_ENTER(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_CHOICE(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
    EVT_BUTTON(wxID_ADD,CslDlgAddMaster::OnCommandEvent)
    EVT_RADIOBUTTON(wxID_ANY,CslDlgAddMaster::OnCommandEvent)
END_EVENT_TABLE()

enum
{
    CHOICE_CTRL_TYPE   = wxID_HIGHEST + 1,
    TEXT_CTRL_ADDRESS,
    TEXT_CTRL_PATH,
    RADIO_CTRL_CUSTOM,
    RADIO_CTRL_DEFAULT
};


CslDlgAddMaster::CslDlgAddMaster(wxWindow* parent,CslMaster *master,int id,
                                 const wxString& title,const wxPoint& pos,
                                 const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style),
        m_master(master)
{
    // begin wxGlade: CslDlgAddMaster::CslDlgAddMaster
    sizer_address_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString choice_type_choices[] =
    {
        _("default")
    };
    choice_type = new wxChoice(this, CHOICE_CTRL_TYPE, wxDefaultPosition, wxDefaultSize, 1, choice_type_choices, 0);
    text_ctrl_address = new wxTextCtrl(this, TEXT_CTRL_ADDRESS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    text_ctrl_path = new wxTextCtrl(this, TEXT_CTRL_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    radio_btn_custom = new wxRadioButton(this, RADIO_CTRL_CUSTOM, _("C&ustom"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    radio_btn_default = new wxRadioButton(this, RADIO_CTRL_DEFAULT, _("Add default &master"));
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
    choice_type->SetSelection(0);
    text_ctrl_address->SetMinSize(wxSize(250,-1));
    text_ctrl_address->SetFocus();
    button_add->Enable(false);
    button_add->SetDefault();
    // end wxGlade

    choice_type->Clear();
    choice_type->Append(CSL_DEFAULT_NAME_SB,(void*)CSL_GAME_SB);
    choice_type->Append(CSL_DEFAULT_NAME_AC,(void*)CSL_GAME_AC);
    choice_type->Append(CSL_DEFAULT_NAME_BF,(void*)CSL_GAME_BF);
    choice_type->Append(CSL_DEFAULT_NAME_CB,(void*)CSL_GAME_CB);
    choice_type->SetSelection(m_master->GetType()==CSL_GAME_START ? CSL_GAME_SB : m_master->GetType()-1);
}

void CslDlgAddMaster::do_layout()
{
    // begin wxGlade: CslDlgAddMaster::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_address = new wxStaticBoxSizer(sizer_address_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_address = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_radio = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_address_top = new wxFlexGridSizer(3, 2, 0, 0);
    wxStaticText* label_game_static = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_address_top->Add(label_game_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->Add(choice_type, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_address_static = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_address_top->Add(label_address_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->Add(text_ctrl_address, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_path_static = new wxStaticText(this, wxID_ANY, _("Path:"));
    grid_sizer_address_top->Add(label_path_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->Add(text_ctrl_path, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address_top->AddGrowableCol(1);
    grid_sizer_address->Add(grid_sizer_address_top, 1, wxTOP|wxEXPAND, 2);
    grid_sizer_radio->Add(radio_btn_custom, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_radio->Add(radio_btn_default, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_radio->AddGrowableCol(0);
    grid_sizer_radio->AddGrowableCol(1);
    grid_sizer_address->Add(grid_sizer_radio, 1, wxEXPAND, 0);
    grid_sizer_address->AddGrowableCol(0);
    sizer_address->Add(grid_sizer_address, 1, wxEXPAND, 0);
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

void CslDlgAddMaster::OnText(wxCommandEvent& WXUNUSED(event))
{
    bool enable=text_ctrl_address->GetValue().IsEmpty() ||
                text_ctrl_path->GetValue().IsEmpty();

    button_add->Enable(!enable);
}

void CslDlgAddMaster::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case CHOICE_CTRL_TYPE:
        {
            // dont break if default is set
            if (!radio_btn_default->GetValue())
                break;
        }

        case RADIO_CTRL_DEFAULT:
        {
            wxString addr,path;
            CSL_GAMETYPE type=(CSL_GAMETYPE)(long)choice_type->GetClientData(choice_type->GetSelection());
            switch (type)
            {
                case CSL_GAME_SB:
                    addr=CSL_DEFAULT_MASTER_SB;
                    path=CSL_DEFAULT_MASTER_PATH_SB;
                    break;
                case CSL_GAME_AC:
                    addr=CSL_DEFAULT_MASTER_AC;
                    path=CSL_DEFAULT_MASTER_PATH_AC;
                    break;
                case CSL_GAME_BF:
                    addr=CSL_DEFAULT_MASTER_BF;
                    path=CSL_DEFAULT_MASTER_PATH_BF;
                    break;
                case CSL_GAME_CB:
                    addr=CSL_DEFAULT_MASTER_CB;
                    path=CSL_DEFAULT_MASTER_PATH_CB;
                    break;
                default:
                    return;
            }
            text_ctrl_address->SetValue(addr);
            text_ctrl_path->SetValue(path);
            text_ctrl_address->Enable(false);
            text_ctrl_path->Enable(false);
            break;
        }

        case RADIO_CTRL_CUSTOM:
            text_ctrl_address->Enable();
            text_ctrl_path->Enable();
            break;

        case TEXT_CTRL_ADDRESS:
        case TEXT_CTRL_PATH:
        case wxID_ADD:
        {
            // check again fore empty host - since setting a default button on wxMAC
            // and hitting enter reaches this position, also if the the button was disabled
            if (text_ctrl_address->GetValue().IsEmpty() ||
                text_ctrl_path->GetValue().IsEmpty())
                return;

            CSL_GAMETYPE type=(CSL_GAMETYPE)(long)choice_type->GetClientData(choice_type->GetSelection());
            wxString addr=text_ctrl_address->GetValue();
            wxString path=text_ctrl_path->GetValue();
            if (!path.StartsWith(wxT("/")))
                path=wxT("/")+path;
            m_master->Create(type,addr,path);
            EndModal(wxID_OK);
        }
        default:
            break;
    }
}
