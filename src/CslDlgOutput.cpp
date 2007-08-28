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

#include "CslDlgOutput.h"

#if 0
CslDlgOutput::CslDlgOutput(wxWindow* parent,int id,const wxString& title,
                           const wxPoint& pos,const wxSize& size, long style):
        wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
{
    // begin wxGlade: CslDlgOutput::CslDlgOutput
    text_ctrl_output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxTE_RICH2);
    checkbox_conversation = new wxCheckBox(this, wxID_ANY, _("Filter conversation (Beta)"));
    button_save = new wxButton(this, wxID_SAVE, _("&Save"));
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgOutput::set_properties()
{
    // begin wxGlade: CslDlgOutput::set_properties
    SetTitle(_("CSL - Game output"));
    // end wxGlade
}


void CslDlgOutput::do_layout()
{
    // begin wxGlade: CslDlgOutput::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(3, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    grid_sizer_main->Add(text_ctrl_output, 0, wxALL|wxEXPAND, 4);
    grid_sizer_main->Add(checkbox_conversation, 0, wxALL, 4);
    grid_sizer_button->Add(1, 1, 0, wxADJUST_MINSIZE, 0);
    grid_sizer_button->Add(button_save, 0, wxALL, 4);
    grid_sizer_button->Add(button_close, 0, wxALL, 4);
    grid_sizer_button->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxALIGN_RIGHT, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade
}
#endif
