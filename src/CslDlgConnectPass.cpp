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
#include "CslDlgConnectPass.h"

BEGIN_EVENT_TABLE(CslDlgConnectPass,wxDialog)
    EVT_BUTTON(wxID_ANY,CslDlgConnectPass::OnCommandEvent)
    EVT_TEXT(wxID_ANY,CslDlgConnectPass::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslDlgConnectPass::OnCommandEvent)
END_EVENT_TABLE()

enum { TEXT_PASSWORD = wxID_HIGHEST + 1, CHECK_ADMIN };


CslDlgConnectPass::CslDlgConnectPass(wxWindow* parent, CslConnectPassInfo *info):
        wxDialog(parent, wxID_ANY, wxString(wxEmptyString)),
        m_info(info)
{
    // begin wxGlade: CslDlgConnectPass::CslDlgConnectPass
    sizer_ctrl_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    m_tcPassword = new wxTextCtrl(this, TEXT_PASSWORD, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    m_cbAdmin = new wxCheckBox(this, CHECK_ADMIN, _("Ad&min connect"));
    m_bsDlg = CreateButtonSizer(wxOK|wxCANCEL);

    set_properties();
    do_layout();
    // end wxGlade
}

void CslDlgConnectPass::set_properties()
{
    // begin wxGlade: CslDlgConnectPass::set_properties
    SetTitle(_("CSL - Server password"));
    m_tcPassword->SetFocus();
    // end wxGlade

    m_tcPassword->SetValue(m_info->Password);

    CSL_SET_WINDOW_ICON();
}

void CslDlgConnectPass::do_layout()
{
    // begin wxGlade: CslDlgConnectPass::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_ctrl = new wxStaticBoxSizer(sizer_ctrl_staticbox, wxVERTICAL);
    wxFlexGridSizer* grid_sizer_password = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticText* label_password = new wxStaticText(this, wxID_ANY, _("Password:"));
    grid_sizer_password->Add(label_password, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_password->Add(m_tcPassword, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_password->AddGrowableCol(1);
    sizer_ctrl->Add(grid_sizer_password, 1, wxEXPAND, 0);
    sizer_ctrl->Add(m_cbAdmin, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_main->Add(sizer_ctrl, 1, wxALL|wxEXPAND, 4);
    grid_sizer_main->Add(m_bsDlg, 1, wxALL|wxEXPAND, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    if (!m_info->Admin)
    {
        m_cbAdmin->Hide();
        sizer_ctrl->Detach(m_cbAdmin);
        grid_sizer_main->Fit(this);
        Layout();
    }

    CSL_CENTRE_DIALOG();
}

void CslDlgConnectPass::OnCommandEvent(wxCommandEvent& event)
{
    wxButton *buttonOK = (wxButton*)FindWindowById(wxID_OK, this);

    switch (event.GetId())
    {
        case TEXT_PASSWORD:
            buttonOK->Enable(!event.GetString().IsEmpty());
            break;

        case CHECK_ADMIN:
            m_tcPassword->SetValue(event.IsChecked() ? m_info->AdminPassword : m_info->Password);
            m_tcPassword->SetFocus();
            break;

        case wxID_OK:
            m_info->Admin = m_cbAdmin->IsChecked();
            if (m_info->Admin)
                m_info->AdminPassword = m_tcPassword->GetValue();
            else
                m_info->Password = m_tcPassword->GetValue();
            EndModal(wxID_OK);
            break;

        default:
            break;
    }

    event.Skip();
}
