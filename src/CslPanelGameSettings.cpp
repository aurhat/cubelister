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
#include "CslGame.h"
#include "CslPanelGameSettings.h"

// begin wxGlade: ::extracode

// end wxGlade

enum
{
    CHECKBOX_STD = wxID_HIGHEST + 1,
    BUTTON_GUESS,
    FILEDIR_EXE,
    FILEDIR_GAME,
    FILEDIR_USER
};

BEGIN_EVENT_TABLE(CslPanelGameSettings, wxPanel)
    EVT_CHECKBOX(wxID_ANY, CslPanelGameSettings::OnCommandEvent)
    EVT_BUTTON(wxID_ANY, CslPanelGameSettings::OnCommandEvent)
    EVT_FILEPICKER_CHANGED(wxID_ANY, CslPanelGameSettings::OnFileDirPicker)
    EVT_DIRPICKER_CHANGED(wxID_ANY, CslPanelGameSettings::OnFileDirPicker)
END_EVENT_TABLE()

CslPanelGameSettings::CslPanelGameSettings(wxWindow* parent, CslGame *game):
    wxPanel(parent, wxID_ANY),
    m_game(game)
{
    // begin wxGlade: CslPanelGameSettings::CslPanelGameSettings
    m_label_game_exe = new wxStaticText(this, wxID_ANY, _("Game executable"));
    m_filepicker_binary = new wxFilePickerCtrl(this, FILEDIR_EXE, m_settings.GamePath, _("Select game installation path"), CSL_EXE_EXTENSIONS, wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL|wxFLP_FILE_MUST_EXIST);
    m_dirpicker_gamepath = new wxDirPickerCtrl(this, FILEDIR_GAME, m_settings.GamePath, _("Select game installation path"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    m_label_userdir = new wxStaticText(this, wxID_ANY, _("Game userdir"));
    m_dirpicker_userdir = new wxDirPickerCtrl(this, FILEDIR_USER, m_settings.GamePath, _("Select game installation path"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL|wxDIRP_DIR_MUST_EXIST);
    m_textctrl_options = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_textctrl_precscript = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_textctrl_postscript = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_checkbox_std = new wxCheckBox(this, CHECKBOX_STD, _("Standard package"));
    m_button_guess = new wxButton(this, BUTTON_GUESS, _("Guess installation path"));

    set_properties();
    do_layout();
    // end wxGlade
}

void CslPanelGameSettings::set_properties()
{
    // begin wxGlade: CslPanelGameSettings::set_properties
    // end wxGlade
}

void CslPanelGameSettings::do_layout()
{
    // begin wxGlade: CslPanelGameSettings::do_layout
    wxFlexGridSizer* gridsizer_main = new wxFlexGridSizer(7, 2, 0, 0);
    gridsizer_main->Add(m_label_game_exe, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_filepicker_binary, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_install_path = new wxStaticText(this, wxID_ANY, _("Game path"));
    gridsizer_main->Add(label_install_path, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_dirpicker_gamepath, 1, wxALL|wxEXPAND, 4);
    gridsizer_main->Add(m_label_userdir, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_dirpicker_userdir, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_params = new wxStaticText(this, wxID_ANY, _("Game parameters"));
    gridsizer_main->Add(label_params, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_textctrl_options, 0, wxALL|wxEXPAND, 4);
    wxStaticText* label_prescript = new wxStaticText(this, wxID_ANY, _("Pre connect script"));
    gridsizer_main->Add(label_prescript, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_textctrl_precscript, 0, wxALL|wxEXPAND, 4);
    wxStaticText* label_postscript = new wxStaticText(this, wxID_ANY, _("Pre connect script"));
    gridsizer_main->Add(label_postscript, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_textctrl_postscript, 0, wxALL|wxEXPAND, 4);
    gridsizer_main->Add(m_checkbox_std, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    gridsizer_main->Add(m_button_guess, 0, wxALL, 4);
    SetSizer(gridsizer_main);
    gridsizer_main->Fit(this);
    gridsizer_main->AddGrowableCol(1);
    // end wxGlade
}

CslPanelGameSettings::~CslPanelGameSettings()
{
    delete m_filepicker_binary;
    delete m_dirpicker_gamepath;
    delete m_dirpicker_userdir;
}

void CslPanelGameSettings::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case CHECKBOX_STD:
        {
            wxWindow *windows[] = { m_filepicker_binary, m_dirpicker_userdir };
            EnableWindows(windows, !event.IsChecked());
            break;
        }
        case BUTTON_GUESS:
            break;
    }
}

void CslPanelGameSettings::OnFileDirPicker(wxFileDirPickerEvent& event)
{
    switch (event.GetId())
    {
        case FILEDIR_EXE:
            if (m_dirpicker_gamepath->GetPath().IsEmpty())
                m_dirpicker_gamepath->SetPath(::wxPathOnly(event.GetPath()));
            if (m_dirpicker_userdir->GetPath().IsEmpty())
                m_dirpicker_userdir->SetPath(::wxPathOnly(event.GetPath()));
            break;
        case FILEDIR_GAME:
            if (m_dirpicker_gamepath->GetPath().IsEmpty())
                m_dirpicker_gamepath->SetPath(event.GetPath());
            if (m_dirpicker_userdir->GetPath().IsEmpty())
                m_dirpicker_userdir->SetPath(event.GetPath());
            break;
        case FILEDIR_USER:
            if (m_dirpicker_userdir->GetPath().IsEmpty())
                m_dirpicker_userdir->SetPath(event.GetPath());
            break;
    }
}

bool CslPanelGameSettings::SaveSettings(wxString& error)
{
    wxString gamepath=m_dirpicker_gamepath->GetPath();
    wxString configpath=m_dirpicker_userdir->GetPath();

    if (!gamepath.IsEmpty() && !gamepath.EndsWith(wxString(CSL_PATHDIV_WX)))
        gamepath+=CSL_PATHDIV_WX;
    if (!configpath.IsEmpty() && !configpath.EndsWith(wxString(CSL_PATHDIV_WX)))
        configpath+=CSL_PATHDIV_WX;

    CslGameClientSettings settings(m_filepicker_binary->GetPath(),gamepath,configpath,
                                   m_textctrl_options->GetValue(),
                                   m_textctrl_precscript->GetValue(),
                                   m_textctrl_postscript->GetValue());

    if (!(error = m_game->ValidateClientSettings(settings)).IsEmpty())
        return false;

    m_game->SetClientSettings(settings);

    return true;
}
