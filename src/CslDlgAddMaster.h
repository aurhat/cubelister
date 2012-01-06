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

#ifndef CSLDLGADDMASTER_H
#define CSLDLGADDMASTER_H

/**
 @author Glen Masgai <mimosius@users.sourceforge.net>
*/


class CslDlgAddMaster: public wxDialog
{
    public:
        // begin wxGlade: CslDlgAddMaster::ids
        // end wxGlade
        CslDlgAddMaster(wxWindow* parent, wxInt32 id=wxID_ANY, const wxString& title=wxEmptyString,
                        const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                        long style=wxDEFAULT_DIALOG_STYLE);

        void InitDlg(wxUint32 *fourcc);

    private:
        // begin wxGlade: CslDlgAddMaster::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void OnCommandEvent(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
        // begin wxGlade: CslDlgAddMaster::attributes
        wxStaticBox* sizer_address_staticbox;
        wxChoice* choice_gametype;
        wxChoice* choice_mastertype;
        wxTextCtrl* text_ctrl_address;
        wxSpinCtrl* spin_ctrl_port;
        wxTextCtrl* text_ctrl_path;
        wxButton* button_add;
        wxButton* button_cancel;
        // end wxGlade

        wxUint32 *m_fourcc;

        bool IsValid();
}; // wxGlade: end class


#endif // CSLDLGADDMASTER_H
