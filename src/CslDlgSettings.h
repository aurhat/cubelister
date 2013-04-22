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

#ifndef CSLDLGSETTINGS_H
#define CSLDLGSETTINGS_H

#include "CslSettings.h"

class CslDlgSettings : public wxDialog
{
    public:
        // begin wxGlade: CslDlgSettings::ids
        // end wxGlade
        CslDlgSettings(wxWindow* parent,int id=wxID_ANY,
                       const wxString& title=wxEmptyString,const wxPoint& pos=wxDefaultPosition,
                       const wxSize& size=wxDefaultSize,long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    private:
        // begin wxGlade: CslDlgSettings::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        CslSettings m_settings;

        void OnSpinCtrl(wxSpinEvent& event);
        void OnPicker(wxFileDirPickerEvent& event);
        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslDlgSettings::attributes
        wxStaticBox* sizer_output_staticbox;
        wxStaticBox* sizer_geoip_staticbox;
        wxStaticBox* sizer_systray_staticbox;
        wxStaticBox* sizer_ping_staticbox;
        wxStaticBox* sizer_tts_staticbox;
        wxStaticBox* sizer_times_staticbox;
        wxStaticBox* sizer_colours_player_staticbox;
        wxStaticBox* sizer_colours_server_staticbox;
        wxListbook* notebook_games;
        wxPanel* notebook_pane_games;
        wxBitmapButton* button_colour_server_empty;
        wxBitmapButton* button_colour_server_mm1;
        wxBitmapButton* button_colour_server_offline;
        wxBitmapButton* button_colour_server_mm2;
        wxBitmapButton* button_colour_server_full;
        wxBitmapButton* button_colour_server_mm3;
        wxBitmapButton* button_colour_player_master;
        wxBitmapButton* button_colour_player_auth;
        wxBitmapButton* button_colour_player_admin;
        wxBitmapButton* button_colour_player_spectator;
        wxPanel* notebook_pane_colour;
        wxSpinCtrl* spin_ctrl_update;
        wxCheckBox* checkbox_play_update;
        wxSpinCtrl* spin_ctrl_wait;
        wxSpinCtrl* spin_ctrl_min_playtime;
        wxSpinCtrl* spin_ctrl_server_cleanup;
        wxCheckBox* checkbox_server_cleanup_favourites;
        wxCheckBox* checkbox_server_cleanup_stats;
        wxSpinCtrl* spin_ctrl_tooltip_delay;
        wxCheckBox* checkbox_tts;
        wxStaticText* label_tts_volume;
        wxSpinCtrl* spin_ctrl_tts_volume;
        wxButton* button_test_tts;
        wxSpinCtrl* spin_ctrl_ping_good;
        wxSpinCtrl* spin_ctrl_ping_bad;
        wxCheckBox* checkbox_systray;
        wxCheckBox* checkbox_systray_close;
        wxRadioButton* radio_btn_geoip_country;
        wxRadioButton* radio_btn_geoip_city;
        wxCheckBox* checkbox_geoip_update;
        wxCheckBox* checkbox_game_output;
        wxDirPickerCtrl* dirpicker_game_output;
        wxPanel* notebook_pane_other;
        wxNotebook* notebook_settings;
        wxSizer* m_bsDlg;
        // end wxGlade

        void SetButtonColour(wxBitmapButton *button, wxButton *refButton, const wxColour& colour);
        bool ValidateSettings();
}; // wxGlade: end class


#endif // CSLDLGSETTINGS_H
