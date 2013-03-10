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
#include "CslEngine.h"
#include "CslPanelTraffic.h"

// begin wxGlade: ::extracode

// end wxGlade


CslPanelTraffic::CslPanelTraffic(wxWindow* parent) :
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL)
{
    // begin wxGlade: CslPanelTraffic::CslPanelTraffic
    label_in_packet = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_in = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_in_raw = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_out_packet = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_out = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_out_raw = new wxStaticText(this, wxID_ANY, wxEmptyString);

    set_properties();
    do_layout();
    // end wxGlade
}

void CslPanelTraffic::set_properties()
{
    // begin wxGlade: CslPanelTraffic::set_properties
    // end wxGlade
}

void CslPanelTraffic::do_layout()
{
    // begin wxGlade: CslPanelTraffic::do_layout
    wxFlexGridSizer* grid_sizer_data = new wxFlexGridSizer(3, 4, 0, 0);
    grid_sizer_data->Add(1, 1, 0, 0, 0);
    wxStaticText* label_packets_static = new wxStaticText(this, wxID_ANY, _("Packets"));
    grid_sizer_data->Add(label_packets_static, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_data_static = new wxStaticText(this, wxID_ANY, _("Data"));
    grid_sizer_data->Add(label_data_static, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_raw_static = new wxStaticText(this, wxID_ANY, _("Data (raw)"));
    grid_sizer_data->Add(label_raw_static, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_in_static = new wxStaticText(this, wxID_ANY, _("Input"));
    grid_sizer_data->Add(label_in_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_in_packet, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_in, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_in_raw, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_out_static = new wxStaticText(this, wxID_ANY, _("Output"));
    grid_sizer_data->Add(label_out_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_out_packet, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_out, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_out_raw, 0, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 4);
    SetSizer(grid_sizer_data);
    grid_sizer_data->Fit(this);
    grid_sizer_data->AddGrowableCol(1);
    grid_sizer_data->AddGrowableCol(2);
    // end wxGlade

    wxFont font = label_packets_static->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);

    label_in_static->SetFont(font);
    label_out_static->SetFont(font);
    label_packets_static->SetFont(font);
    label_data_static->SetFont(font);
    label_raw_static->SetFont(font);

    wxSize size = GetBestSize();
    size.x *= 1.3f;

    SetSize(size);
    SetMinSize(size);
}

void CslPanelTraffic::UpdateStats()
{
    static wxUint32 ticks = 0;

    if (++ticks%4 || !IsShown())
        return;

    wxWindowUpdateLocker lock(this);

    label_in_packet->SetLabel(wxString::Format(wxT("%d"), CslUDP::GetPacketCount(CSL_UDP_TRAFFIC_IN)));
    label_out_packet->SetLabel(wxString::Format(wxT("%d"), CslUDP::GetPacketCount(CSL_UDP_TRAFFIC_OUT)));
    label_in->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_IN, true)));
    label_out->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_OUT, true)));
    label_in_raw->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_IN)));
    label_out_raw->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_OUT)));
}
