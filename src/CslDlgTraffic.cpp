/***************************************************************************
 *   Copyright (C) 2007-2009 by Glen Masgai                                *
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
#include "engine/CslEngine.h"
#include "CslDlgTraffic.h"


BEGIN_EVENT_TABLE(CslDlgTraffic,wxDialog)
    EVT_TIMER(wxID_ANY,CslDlgTraffic::OnTimer)
    EVT_BUTTON(wxID_ANY,CslDlgTraffic::OnCommandEvent)
    EVT_CLOSE(CslDlgTraffic::OnClose)
END_EVENT_TABLE()


CslDlgTraffic::CslDlgTraffic(wxWindow* parent,int id,const wxString& title,
                             const wxPoint& pos,const wxSize& size,long style):
        wxDialog(parent,id,title,pos,size,style)
{
    // begin wxGlade: CslDlgTraffic::CslDlgTraffic
    sizer_main_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    label_in_packet = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_in = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_in_raw = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_out_packet = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_out = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_out_raw = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade
}


void CslDlgTraffic::set_properties()
{
    // begin wxGlade: CslDlgTraffic::set_properties
    SetTitle(_("CSL - Traffic statistics"));
    button_close->SetDefault();
    // end wxGlade
    UpdateStats();
}


void CslDlgTraffic::do_layout()
{
    // begin wxGlade: CslDlgTraffic::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxStaticBoxSizer* sizer_main = new wxStaticBoxSizer(sizer_main_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_data = new wxFlexGridSizer(3, 4, 0, 0);
    grid_sizer_data->Add(1, 1, 0, 0, 0);
    wxStaticText* label_packets_static = new wxStaticText(this, wxID_ANY, _("Packets"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_packets_static->SetMinSize(wxSize(80,-1));
    grid_sizer_data->Add(label_packets_static, 0, wxALL|wxEXPAND, 4);
    wxStaticText* label_data_static = new wxStaticText(this, wxID_ANY, _("Data"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_data_static->SetMinSize(wxSize(80,-1));
    grid_sizer_data->Add(label_data_static, 0, wxALL|wxEXPAND, 4);
    wxStaticText* label_raw_static = new wxStaticText(this, wxID_ANY, _("Data (raw)"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    label_raw_static->SetMinSize(wxSize(80,-1));
    grid_sizer_data->Add(label_raw_static, 0, wxALL|wxEXPAND, 4);
    wxStaticText* label_in_static = new wxStaticText(this, wxID_ANY, _("Input"));
    grid_sizer_data->Add(label_in_static, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_in_packet, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_in, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_data->Add(label_in_raw, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 4);
    wxStaticText* label_out_static = new wxStaticText(this, wxID_ANY, _("Output"));
    grid_sizer_data->Add(label_out_static, 0, wxALL, 4);
    grid_sizer_data->Add(label_out_packet, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 4);
    grid_sizer_data->Add(label_out, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 4);
    grid_sizer_data->Add(label_out_raw, 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 4);
    grid_sizer_data->AddGrowableRow(2);
    grid_sizer_data->AddGrowableCol(1);
    grid_sizer_data->AddGrowableCol(2);
    grid_sizer_data->AddGrowableCol(3);
    sizer_main->Add(grid_sizer_data, 1, wxEXPAND, 0);
    grid_sizer_main->Add(sizer_main, 1, wxALL|wxEXPAND, 4);
    grid_sizer_main->Add(button_close, 0, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 8);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade
    grid_sizer_main->SetSizeHints(this);

    CentreOnParent();
}

void CslDlgTraffic::OnTimer(wxTimerEvent& event)
{
    static wxUint32 count=0;
    if (++count%4==0)
    {
        UpdateStats();
        Layout();
    }
}

void CslDlgTraffic::OnCommandEvent(wxCommandEvent& event)
{
    if (event.GetId()==wxID_CLOSE) Close();
    event.Skip();
}

void CslDlgTraffic::OnClose(wxCloseEvent& event)
{
    Hide();
    wxPostEvent(GetParent(),event);
}

void CslDlgTraffic::UpdateStats()
{
    label_in_packet->SetLabel(wxString::Format(wxT("%d"),CslUDP::GetPacketCount(CSL_UDP_TRAFFIC_IN)));
    label_out_packet->SetLabel(wxString::Format(wxT("%d"),CslUDP::GetPacketCount(CSL_UDP_TRAFFIC_OUT)));
    label_in->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_IN,true)));
    label_out->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_OUT,true)));
    label_in_raw->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_IN)));
    label_out_raw->SetLabel(FormatBytes(CslUDP::GetTraffic(CSL_UDP_TRAFFIC_OUT)));
}
