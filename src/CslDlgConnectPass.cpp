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

#include "CslDlgConnectPass.h"

BEGIN_EVENT_TABLE(CslDlgConnectPass,wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgConnectPass::OnCommandEvent)
    EVT_TEXT(wxID_ANY,CslDlgConnectPass::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslDlgConnectPass::OnCommandEvent)
END_EVENT_TABLE()

enum { TEXT_PASSWORD = wxID_HIGHEST + 1, CHECK_ADMIN };


CslDlgConnectPass::CslDlgConnectPass(wxWindow* parent,CslConnectPassInfo *info,int id,
                                     const wxString& title,const wxPoint& pos,
                                     const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,wxDEFAULT_DIALOG_STYLE), m_info(info)
{
    // begin wxGlade: CslDlgConnectPass::CslDlgConnectPass
    sizer_ctrl_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    text_ctrl_password = new wxTextCtrl(this, TEXT_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    checkbox_admin = new wxCheckBox(this, CHECK_ADMIN, _("Ad&min connect"));
    button_ok = new wxButton(this, wxID_OK, _("&Ok"));
    button_cancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

    set_properties();
    do_layout();
    // end wxGlade
}

void CslDlgConnectPass::set_properties()
{
    // begin wxGlade: CslDlgConnectPass::set_properties
    SetTitle(_("CSL - Server password"));
    text_ctrl_password->SetFocus();
    button_ok->Enable(false);
    button_ok->SetDefault();
    // end wxGlade

    if (!m_info->Password.IsEmpty())
        text_ctrl_password->SetValue(m_info->Password);
}

void CslDlgConnectPass::do_layout()
{
    // begin wxGlade: CslDlgConnectPass::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_ctrl = new wxStaticBoxSizer(sizer_ctrl_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_ctrl = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_password = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticText* label_password_static = new wxStaticText(this, wxID_ANY, _("Password:"));
    grid_sizer_password->Add(label_password_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_password->Add(text_ctrl_password, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_password->AddGrowableCol(1);
    grid_sizer_ctrl->Add(grid_sizer_password, 1, wxEXPAND, 0);
    grid_sizer_ctrl->Add(checkbox_admin, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_ctrl->AddGrowableCol(0);
    sizer_ctrl->Add(grid_sizer_ctrl, 1, wxEXPAND, 0);
    grid_sizer_main->Add(sizer_ctrl, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(1, 1, 0, 0, 0);
    grid_sizer_button->Add(button_ok, 0, wxALL, 4);
    grid_sizer_button->Add(button_cancel, 0, wxALL, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    Layout();
    // end wxGlade

    if (!m_info->Admin)
    {
        checkbox_admin->Hide();
        grid_sizer_ctrl->Detach(checkbox_admin);
        grid_sizer_main->Fit(this);
        Layout();
    }

    CentreOnParent();
}

void CslDlgConnectPass::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case TEXT_PASSWORD:
            button_ok->Enable(!event.GetString().IsEmpty());
            break;

        case CHECK_ADMIN:
            text_ctrl_password->SetValue(event.IsChecked() ? m_info->AdminPassword:m_info->Password);
            text_ctrl_password->SetFocus();
            break;

        case wxID_OK:
            m_info->Admin=checkbox_admin->IsChecked();
            if (m_info->Admin)
                m_info->AdminPassword=text_ctrl_password->GetValue();
            else
                m_info->Password=text_ctrl_password->GetValue();
            EndModal(wxID_OK);
            break;

        default:
            break;
    }

    event.Skip();
}
