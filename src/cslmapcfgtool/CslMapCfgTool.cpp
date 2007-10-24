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

#include "CslMapCfgTool.h"

// begin wxGlade: ::extracode
// end wxGlade

enum
{
    COMBO_BASE        = wxID_HIGHEST + 1,
    BUTTON_BASE_ADD,
    BUTTON_BASE_DEL,
    BUTTON_LOAD_IMAGE,
    BUTTON_RESET
};


CslMapCfgTool::CslMapCfgTool(wxWindow* parent,int id,const wxString& title,
                             const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
{
    // begin wxGlade: CslMapCfgTool::CslMapCfgTool
    sizer_control_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    sizer_map_staticbox = new wxStaticBox(this, -1, _("Map"));
    panel_bitmap = new wxPanel(this, wxID_ANY);
    const wxString choice_base_choices[] =
    {
        _("0")
    };
    choice_base = new wxChoice(this, COMBO_BASE, wxDefaultPosition, wxDefaultSize, 1, choice_base_choices, 0);
    button_base_add = new wxButton(this, BUTTON_BASE_ADD, _("&Add"));
    button_base_del = new wxButton(this, BUTTON_BASE_DEL, _("Rem&ove"));
    static_line_1 = new wxStaticLine(this, wxID_ANY);
    text_ctrl_map_name = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    text_ctrl_author = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    static_line_2 = new wxStaticLine(this, wxID_ANY);
    button_load_image = new wxButton(this, BUTTON_LOAD_IMAGE, _("Load &image"));
    button_reset = new wxButton(this, BUTTON_RESET, _("&Reset"));
    button_save = new wxButton(this, wxID_SAVE, _("&Save"));
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade
}

void CslMapCfgTool::set_properties()
{
    // begin wxGlade: CslMapCfgTool::set_properties
    SetTitle(_("CSL - Map config tool"));
    panel_bitmap->SetMinSize(wxSize(400, 300));
    panel_bitmap->SetBackgroundColour(wxColour(0, 0, 0));
    choice_base->SetSelection(0);
    // end wxGlade
}

void CslMapCfgTool::do_layout()
{
    // begin wxGlade: CslMapCfgTool::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_top = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_control = new wxStaticBoxSizer(sizer_control_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_control = new wxFlexGridSizer(5, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_other = new wxFlexGridSizer(1, 3, 0, 0);
    wxFlexGridSizer* grid_sizer_desc = new wxFlexGridSizer(2, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_base = new wxFlexGridSizer(1, 4, 0, 0);
    wxStaticBoxSizer* sizer_map = new wxStaticBoxSizer(sizer_map_staticbox, wxHORIZONTAL);
    sizer_map->Add(panel_bitmap, 1, wxEXPAND, 0);
    grid_sizer_top->Add(sizer_map, 1, wxEXPAND, 0);
    wxStaticText* label_bases_static = new wxStaticText(this, wxID_ANY, _("Bases:"));
    grid_sizer_base->Add(label_bases_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_base->Add(choice_base, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_base->Add(button_base_add, 0, wxALL, 4);
    grid_sizer_base->Add(button_base_del, 0, wxALL, 4);
    grid_sizer_control->Add(grid_sizer_base, 1, wxEXPAND, 0);
    grid_sizer_control->Add(static_line_1, 0, wxEXPAND, 0);
    wxStaticText* label_map_name_static = new wxStaticText(this, wxID_ANY, _("Map name:"));
    grid_sizer_desc->Add(label_map_name_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_desc->Add(text_ctrl_map_name, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_author_static = new wxStaticText(this, wxID_ANY, _("Author:"));
    grid_sizer_desc->Add(label_author_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_desc->Add(text_ctrl_author, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_desc->AddGrowableCol(1);
    grid_sizer_control->Add(grid_sizer_desc, 1, wxEXPAND, 0);
    grid_sizer_control->Add(static_line_2, 0, wxEXPAND, 0);
    grid_sizer_other->Add(button_load_image, 0, wxALL, 4);
    grid_sizer_other->Add(1, 1, 0, 0, 0);
    grid_sizer_other->Add(button_reset, 0, wxALL, 4);
    grid_sizer_other->AddGrowableCol(1);
    grid_sizer_control->Add(grid_sizer_other, 1, wxEXPAND, 0);
    grid_sizer_control->AddGrowableCol(0);
    sizer_control->Add(grid_sizer_control, 1, wxALL|wxEXPAND, 4);
    grid_sizer_top->Add(sizer_control, 1, wxEXPAND, 0);
    grid_sizer_top->AddGrowableRow(0);
    grid_sizer_top->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_top, 1, wxALL|wxEXPAND, 4);
    grid_sizer_button->Add(button_save, 0, wxALL, 4);
    grid_sizer_button->Add(20, 1, 0, 0, 0);
    grid_sizer_button->Add(button_close, 0, wxALL, 4);
    grid_sizer_main->Add(grid_sizer_button, 1, wxALL|wxALIGN_RIGHT, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade
}


IMPLEMENT_APP(CslMapCfgToolApp)

bool CslMapCfgToolApp::OnInit()
{
    wxInitAllImageHandlers();
    CslMapCfgTool* dlg_cslmapcfgtool=new CslMapCfgTool(NULL,wxID_ANY,wxEmptyString);
    SetTopWindow(dlg_cslmapcfgtool);
    dlg_cslmapcfgtool->Show();
    return true;
}
