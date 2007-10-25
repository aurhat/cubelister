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

#include "CslDlgAddServer.h"

BEGIN_EVENT_TABLE(CslDlgAddServer,wxDialog)
    EVT_TEXT(wxID_ANY,CslDlgAddServer::OnText)
    EVT_BUTTON(wxID_ADD,CslDlgAddServer::OnButton)
    EVT_TEXT_ENTER(wxID_ANY,CslDlgAddServer::OnButton)
END_EVENT_TABLE()

CslDlgAddServer::CslDlgAddServer(wxWindow* parent,CslServerInfo *info,int id,const wxString& title,
                                 const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, style), m_info(info)
{
    // begin wxGlade: CslDlgAddServer::CslDlgAddServer
    sizer_address_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    const wxString choice_type_choices[] =
    {
        _("default")
    };
    choice_type = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, choice_type_choices, 0);
    text_ctrl_address = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
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
    choice_type->SetSelection(0);
}


void CslDlgAddServer::do_layout()
{
    // begin wxGlade: CslDlgAddServer::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_address = new wxStaticBoxSizer(sizer_address_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_address = new wxFlexGridSizer(2, 2, 0, 0);
    wxStaticText* label_game_static = new wxStaticText(this, wxID_ANY, _("Game:"));
    grid_sizer_address->Add(label_game_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(choice_type, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_address_static = new wxStaticText(this, wxID_ANY, _("Address:"));
    grid_sizer_address->Add(label_address_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_address->Add(text_ctrl_address, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
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

void CslDlgAddServer::OnText(wxCommandEvent& WXUNUSED(event))
{
    bool enable=text_ctrl_address->GetValue().IsEmpty();
    button_add->Enable(!enable);
}

void CslDlgAddServer::OnButton(wxCommandEvent& event)
{
    wxString host=text_ctrl_address->GetValue();

    // check again for empty host - since setting a default button on wxMAC
    // and hitting enter reaches this position, also if the the button was disabled
    if (host.IsEmpty())
        return;

    CSL_GAMETYPE type=(CSL_GAMETYPE)(long)choice_type->GetClientData(choice_type->GetSelection());

    m_info->CreateFavourite(host,type);

    EndModal(wxID_OK);
}
