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

#ifndef CSLDLGCONNECTWAIT_H
#define CSLDLGCONNECTWAIT_H

class CslDlgConnectWait: public wxDialog
{
    public:
    // begin wxGlade: CslDlgConnectWait::ids
    // end wxGlade

        CslDlgConnectWait(wxWindow* parent,wxInt32 *time,int id=wxID_ANY,const wxString& title=wxEmptyString,
                          const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                          long style=wxDEFAULT_DIALOG_STYLE);

    private:
    // begin wxGlade: CslDlgConnectWait::methods
    void set_properties();
    void do_layout();
    // end wxGlade

        wxInt32 *m_time;
        void OnButton(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()

    protected:
    // begin wxGlade: CslDlgConnectWait::attributes
    wxSpinCtrl* spin_ctrl_time;
    wxCheckBox* checkbox_default;
    wxButton* button_cancel;
    wxButton* button_ok;
    wxButton* button_connect;
    // end wxGlade
}; // wxGlade: end class


#endif // CSLDLGCONNECTWAIT_H
