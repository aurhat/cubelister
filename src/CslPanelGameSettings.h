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

#ifndef CSLPANELGAMESETTINGS_H
#define CSLPANELGAMESETTINGS_H

// begin wxGlade: ::extracode

// end wxGlade

class CslPanelGameSettings: public wxPanel {
    public:
        // begin wxGlade: CslPanelGameSettings::ids
        // end wxGlade

        CslPanelGameSettings(wxWindow* parent, CslGame *game);
        ~CslPanelGameSettings();

        bool SaveSettings(wxString& error);

    private:
        // begin wxGlade: CslPanelGameSettings::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnCommandEvent(wxCommandEvent& event);
        void OnFileDirPicker(wxFileDirPickerEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslPanelGameSettings::attributes
        wxStaticText* m_label_game_exe;
        wxFilePickerCtrl* m_filepicker_binary;
        wxDirPickerCtrl* m_dirpicker_gamepath;
        wxStaticText* m_label_userdir;
        wxDirPickerCtrl* m_dirpicker_userdir;
        wxTextCtrl* m_textctrl_options;
        wxTextCtrl* m_textctrl_precscript;
        wxTextCtrl* m_textctrl_postscript;
        wxCheckBox* m_checkbox_std;
        wxButton* m_button_guess;
        // end wxGlade

        CslGame *m_game;
        CslGameClientSettings m_settings;
}; // wxGlade: end class


#endif // CSLPANELGAMESETTINGS_H
