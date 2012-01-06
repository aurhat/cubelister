/***************************************************************************
 *   Copyright (C) 2007-2011 by Glen Masgai                                *
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

#ifndef CSLWIZARDGAMESETTINGS_H
#define CSLWIZARDGAMESETTINGS_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include "CslSettings.h"

#ifdef CSL_WIZARD_DEFINE_PAGE_CLASSES
class CslWizardGameSettings;

class CslWizardGameSettingsPagePaths : public wxWizardPageSimple
{
    public:
        CslWizardGameSettingsPagePaths(CslWizardGameSettings *parent);

    private:
        enum { PICK_GAME = wxID_HIGHEST + 1, PICK_EXE, PICK_CFG };

        CslWizardGameSettings *m_parent;
        wxFlexGridSizer *m_sizer;
        CslFilePickerCtrl *m_pick_exe;
        CslDirPickerCtrl *m_pick_dir,*m_pick_cfg;

        void OnPickEvent(CslFileDirPickerEvent& event);

        DECLARE_EVENT_TABLE()
};


class CslWizardGameSettingsPageGFX : public wxWizardPageSimple
{
    public:
        CslWizardGameSettingsPageGFX(CslWizardGameSettings *parent);

    private:
        CslWizardGameSettings *m_parent;

        wxFlexGridSizer *m_sizer,*m_sizer_resolution;
        wxTextCtrl *m_text_width,*m_text_height;
        wxCheckBox *m_check_window_mode;
        wxStaticText *m_static_width,*m_static_height;
        vector<wxRadioButton*> m_radio_buttons;

        void ToggleResolution(wxInt32 id);

        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()
};


class CslWizardGameSettingsPageScripts : public wxWizardPageSimple
{
    public:
        CslWizardGameSettingsPageScripts(CslWizardGameSettings *parent);

    private:
        CslWizardGameSettings *m_parent;

        wxTextCtrl *m_text_pre,*m_text_post;
};


class CslDescriptionSizer : public wxFlexGridSizer
{
    public:
        CslDescriptionSizer(wxWindow *parent, const wxString& title) :
                wxFlexGridSizer(1, 3, 0, 0)
        {
            Add(new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(32, 32)),
                0, wxALIGN_CENTER_VERTICAL);
            Add(new wxStaticText(parent, wxID_ANY, title), 0, wxALIGN_CENTER_VERTICAL);
            Add(new wxStaticLine(parent), 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
            AddGrowableCol(2);
        }
};
#endif //CSL_WIZARD_DEFINE_PAGE_CLASSES


class CslWizardGameSettings : public wxWizard
{
    public:
        CslWizardGameSettings(wxWindow *parent,
                              CslGame *m_game, CslGameClientSettings *settings,
                              const wxString &title, const wxBitmap &bitmap=wxNullBitmap);

        bool Run() { return RunWizard(m_page); }

        CslGame& GetGame() { return *m_game; }
        CslGameClientSettings& GetClientSettings() { return *m_settings; }

    private:
        CslGame *m_game;
        CslGameClientSettings *m_settings;
        wxWizardPageSimple *m_page;
};

#endif //CSLWIZARDGAMESETTINGS_H
