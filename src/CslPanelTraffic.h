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

#ifndef CSLPANELTRAFFIC_H
#define CSLPANELTRAFFIC_H


// begin wxGlade: ::extracode

// end wxGlade


class CslPanelTraffic : public wxPanel {
    public:
        // begin wxGlade: CslPanelTraffic::ids
        // end wxGlade

        CslPanelTraffic(wxWindow* parent);

        void UpdateStats();

    private:
        // begin wxGlade: CslPanelTraffic::methods
        void set_properties();
        void do_layout();
        // end wxGlade

    protected:
        // begin wxGlade: CslPanelTraffic::attributes
        wxStaticText* label_in_packet;
        wxStaticText* label_in;
        wxStaticText* label_in_raw;
        wxStaticText* label_out_packet;
        wxStaticText* label_out;
        wxStaticText* label_out_raw;
        // end wxGlade
}; // wxGlade: end class


#endif // CSLPANELTRAFFIC_H
