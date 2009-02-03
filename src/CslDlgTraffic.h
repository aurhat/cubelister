/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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

#ifndef CSLDLGTRAFFIC_H
#define CSLDLGTRAFFIC_H

/**
    @author Glen Masgai <mimosius@gmx.de>
*/

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif


// begin wxGlade: ::dependencies
// end wxGlade

// begin wxGlade: ::extracode

// end wxGlade


class CslDlgTraffic: public wxDialog
{
    public:
        // begin wxGlade: CslDlgTraffic::ids
        // end wxGlade
        CslDlgTraffic(wxWindow* parent,int id=wxID_ANY,const wxString& title=wxEmptyString,
                      const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,
                      long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    private:
        // begin wxGlade: CslDlgTraffic::methods
        void set_properties();
        void do_layout();
        // end wxGlade

        void UpdateStats();

        void OnTimer(wxTimerEvent& event);
        void OnCommandEvent(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);

        DECLARE_EVENT_TABLE()


    protected:
        // begin wxGlade: CslDlgTraffic::attributes
        wxStaticBox* sizer_main_staticbox;
        wxStaticText* label_in_packet;
        wxStaticText* label_in;
        wxStaticText* label_in_raw;
        wxStaticText* label_out_packet;
        wxStaticText* label_out;
        wxStaticText* label_out_raw;
        wxButton* button_close;
        // end wxGlade
}; // wxGlade: end class


#endif // CSLDLGTRAFFIC_H
