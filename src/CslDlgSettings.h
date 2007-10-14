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

#ifndef CSLDLGSETTINGS_H
#define CSLDLGSETTINGS_H

/**
 @author Glen Masgai <mimosius@gmx.de>
*/

#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/image.h>
// begin wxGlade: ::dependencies
#include <wx/notebook.h>
#include <wx/spinctrl.h>
// end wxGlade
#include <wx/listbook.h>
#include <wx/filepicker.h>
#include "CslSettings.h"

class CslDlgSettings: public wxDialog
{
    public:
        // begin wxGlade: CslDlgSettings::ids
        // end wxGlade
        CslDlgSettings(wxWindow* parent,int id=wxID_ANY,
                       const wxString& title=wxEmptyString,const wxPoint& pos=wxDefaultPosition,
                       const wxSize& size=wxDefaultSize,long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

        ~CslDlgSettings();

    private:
        // begin wxGlade: CslDlgSettings::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        CslSettings m_settings;

        void OnPicker(wxFileDirPickerEvent& event);
        void OnSpinCtrl(wxSpinEvent& event);
        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslDlgSettings::attributes
        wxStaticBox* sizer_ping_threshold_staticbox;
        wxStaticBox* sizer_times_staticbox;
        wxStaticBox* sizer_colours_staticbox;
        wxTextCtrl* text_ctrl_sauer_options;
        wxPanel* notebook_pane_sauer;
        wxTextCtrl* text_ctrl_assault_options;
        wxPanel* notebook_pane_assault;
        wxTextCtrl* text_ctrl_frontier_options;
        wxPanel* notebook_pane_frontier;
        wxTextCtrl* text_ctrl_cube_options;
        wxPanel* notebook_pane_cube;
        wxListbook* notebook_games;
        wxPanel* notebook_pane_games;
        wxBitmapButton* button_colour_empty;
        wxBitmapButton* button_colour_mm1;
        wxBitmapButton* button_colour_off;
        wxBitmapButton* button_colour_mm2;
        wxBitmapButton* button_colour_full;
        wxBitmapButton* button_colour_mm3;
        wxPanel* notebook_pane_colour;
        wxSpinCtrl* spin_ctrl_update;
        wxCheckBox* checkbox_play_update;
        wxSpinCtrl* spin_ctrl_wait;
        wxSpinCtrl* spin_ctrl_min_playtime;
        wxSpinCtrl* spin_ctrl_ping_good;
        wxSpinCtrl* spin_ctrl_ping_bad;
        wxPanel* notebook_pane_other;
        wxNotebook* notebook_settings;
        wxButton* button_ok;
        wxButton* button_cancel;
        // end wxGlade

#ifndef __WXMAC__
        wxFilePickerCtrl *fpickSauer,*fpickAssault,*fpickFrontier,*fpickCube;
#endif
        wxDirPickerCtrl *dpickSauer,*dpickAssault,*dpickFrontier,*dpickCube;

        void SetButtonColour(wxBitmapButton *button,wxButton *refButton,wxColour& colour);
        bool Validate();
}; // wxGlade: end class


#endif // CSLDLGSETTINGS_H
