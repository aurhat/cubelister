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

#include "CslDlgExtended.h"
#include "CslSettings.h"
#include "CslFlags.h"
#ifndef __WXMSW__
#include "img/sortasc_18_12.xpm"
#include "img/sortdsc_18_12.xpm"
#endif


BEGIN_EVENT_TABLE(CslDlgExtended, wxDialog)
    EVT_CLOSE(CslDlgExtended::OnClose)
    EVT_SIZE(CslDlgExtended::OnSize)
    EVT_LIST_COL_CLICK(wxID_ANY,CslDlgExtended::OnColumnLeftClick)
    EVT_TIMER(wxID_ANY,CslDlgExtended::OnTimer)
    EVT_BUTTON(wxID_ANY,CslDlgExtended::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslDlgExtended::OnCommandEvent)
    CSL_EVT_PONG(wxID_ANY,CslDlgExtended::OnPong)
END_EVENT_TABLE()


enum
{
    CSL_LIST_IMG_SORT_ASC = 0,
    CSL_LIST_IMG_SORT_DSC,
    CSL_LIST_IMG_UNKNOWN
};

enum
{
    SORT_PLAYER = 0, SORT_TEAM,
    SORT_FRAGS, SORT_DEATHS, SORT_TEAMKILLS,
    SORT_HEALTH, SORT_ARMOUR, SORT_WEAPON
};

enum { BUTTON_REFRESH = wxID_HIGHEST +1, CHECK_UPDATE, CHECK_MAP };

#define CSL_EXT_REFRESH_INTERVAL  g_cslSettings->m_updateInterval/1000

static const wxColour team_colours[] =
{
    wxColour(0,0,255),wxColour(255,0,0),wxColour(64,160,64),wxColour(192,128,0),
    wxColour(190,60,75),wxColour(60,190,75),wxColour(225,225,60),wxColour(120,50,140)
};


CslDlgExtended::CslDlgExtended(wxWindow* parent,int id,const wxString& title,
                               const wxPoint& pos, const wxSize& size, long style):
        wxDialog(parent, id, title, pos, size, style)
{
    m_engine=NULL;
    m_info=NULL;
    m_gridSizerMain=m_gridSizerList=m_gridSizerInfo=NULL;
    m_sizerMap=m_sizerMapLabel=NULL;

    // begin wxGlade: CslDlgExtended::CslDlgExtended
    sizer_team_score_staticbox = new wxStaticBox(this, -1, _("Team score"));
    sizer_info_staticbox = new wxStaticBox(this, -1, _("Game information"));
    sizer_map_label_staticbox = new wxStaticBox(this, -1, _("Map information"));
    sizer_update_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    sizer_map_staticbox = new wxStaticBox(this, -1, _("Map"));
    list_ctrl_players = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
    panel_map = new CslPanelMap(this, wxID_ANY);
    label_team1 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team2 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team3 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team4 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team5 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team6 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team7 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team8 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_server = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_mode = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_remaining = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_records = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_map = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_author_prefix = new wxStaticText(this, wxID_ANY, _("by"));
    label_author = new wxStaticText(this, wxID_ANY, wxEmptyString);
    button_update = new wxButton(this, BUTTON_REFRESH, _("&Update"));
    checkbox_update = new wxCheckBox(this, CHECK_UPDATE, _("&Auto update"));
    checkbox_update_end = new wxCheckBox(this, wxID_ANY, _("Stop auto update when map &ends"));
    checkbox_map = new wxCheckBox(this, CHECK_MAP, _("&Show map if possible"));
    static_line = new wxStaticLine(this, wxID_ANY);
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade

#ifdef __WXMSW__
    m_imageList.Create(20,14,true);
    m_imageList.Add(AdjustIconSize(NULL,wxIcon(wxT("sortasc_18_12"),wxBITMAP_TYPE_ICO_RESOURCE,18,12),
                                   wxSize(20,14),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(NULL,wxIcon(wxT("sortdsc_18_12"),wxBITMAP_TYPE_ICO_RESOURCE,18,12),
                                   wxSize(20,14),wxPoint(0,0)));
    m_imageList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,2)));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        m_imageList.Add(AdjustIconSize(flags[i],wxNullIcon,wxSize(20,14),wxPoint(0,2)));
#else
    m_imageList.Create(18,12,true);
    m_imageList.Add(wxBitmap(sortasc_18_12_xpm));
    m_imageList.Add(wxBitmap(sortdsc_18_12_xpm));
    m_imageList.Add(wxBitmap(unknown_xpm));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        m_imageList.Add(wxBitmap(flags[i]));
#endif
}

CslDlgExtended::~CslDlgExtended()
{
    if (!panel_map->IsShown())
        delete m_sizerMap;
}

void CslDlgExtended::set_properties()
{
    // begin wxGlade: CslDlgExtended::set_properties
    SetTitle(_("CSL - Extended info"));
    label_team1->SetMinSize(wxSize(60, -1));
    label_team2->SetMinSize(wxSize(60, -1));
    checkbox_update_end->Enable(false);
    button_close->SetDefault();
    // end wxGlade

    m_labelFont=label_team1->GetFont();

    wxInt32 pointSize=m_labelFont.GetPointSize();
    label_map->SetFont(wxFont(pointSize+10,wxDECORATIVE,wxNORMAL,wxBOLD));
    label_author->SetFont(wxFont(pointSize+4,wxDECORATIVE,wxITALIC,wxBOLD));

    m_teamLabel.Add(label_team1);
    m_teamLabel.Add(label_team2);
    m_teamLabel.Add(label_team3);
    m_teamLabel.Add(label_team4);
    m_teamLabel.Add(label_team5);
    m_teamLabel.Add(label_team6);
    m_teamLabel.Add(label_team7);
    m_teamLabel.Add(label_team8);

#ifdef __WXMSW__
    list_ctrl_players->SetMinSize(wxSize(520,300));
#endif
#ifdef __WXGTK__
    list_ctrl_players->SetMinSize(wxSize(580,350));
#endif
#ifdef __WXMAC__
    list_ctrl_players->SetMinSize(wxSize(580,330));
    button_update->SetMinSize(wxSize(button_update->GetSize().x+20,-1));
#endif
}

void CslDlgExtended::do_layout()
{
    // begin wxGlade: CslDlgExtended::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(5, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 1, 0, 0);
    wxStaticBoxSizer* sizer_update = new wxStaticBoxSizer(sizer_update_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_update = new wxFlexGridSizer(1, 5, 0, 0);
    wxFlexGridSizer* grid_sizer_info_team = new wxFlexGridSizer(1, 3, 0, 0);
    wxStaticBoxSizer* sizer_map_label = new wxStaticBoxSizer(sizer_map_label_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_map_label = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_author = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_info = new wxStaticBoxSizer(sizer_info_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_info = new wxFlexGridSizer(4, 2, 0, 0);
    wxStaticBoxSizer* sizer_team_score = new wxStaticBoxSizer(sizer_team_score_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_team_score = new wxFlexGridSizer(4, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_list_map = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_map = new wxStaticBoxSizer(sizer_map_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_map = new wxFlexGridSizer(3, 1, 0, 0);
    grid_sizer_list_map->Add(list_ctrl_players, 1, wxALL|wxEXPAND, 4);
    grid_sizer_map->Add(1, 1, 0, 0, 0);
    grid_sizer_map->Add(panel_map, 1, wxEXPAND, 0);
    grid_sizer_map->Add(1, 1, 0, 0, 0);
    grid_sizer_map->AddGrowableRow(0);
    grid_sizer_map->AddGrowableRow(2);
    grid_sizer_map->AddGrowableCol(0);
    sizer_map->Add(grid_sizer_map, 1, wxEXPAND, 0);
    grid_sizer_list_map->Add(sizer_map, 1, wxALL|wxEXPAND, 3);
    grid_sizer_list_map->AddGrowableRow(0);
    grid_sizer_list_map->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_list_map, 1, wxEXPAND, 0);
    grid_sizer_team_score->Add(label_team1, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team2, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team3, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team4, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team5, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team6, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team7, 0, wxALL, 4);
    grid_sizer_team_score->Add(label_team8, 0, wxALL, 4);
    sizer_team_score->Add(grid_sizer_team_score, 1, wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_info_team->Add(sizer_team_score, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    wxStaticText* label_server_static = new wxStaticText(this, wxID_ANY, _("Server:"));
    grid_sizer_info->Add(label_server_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_server, 0, wxALL, 4);
    wxStaticText* label_mode_static = new wxStaticText(this, wxID_ANY, _("Mode:"));
    grid_sizer_info->Add(label_mode_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_mode, 0, wxALL, 4);
    wxStaticText* label_remaining_static = new wxStaticText(this, wxID_ANY, _("Time remaining:"));
    grid_sizer_info->Add(label_remaining_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_remaining, 0, wxALL, 4);
    wxStaticText* label_records_static = new wxStaticText(this, wxID_ANY, _("Player records:"));
    grid_sizer_info->Add(label_records_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_records, 0, wxALL, 4);
    sizer_info->Add(grid_sizer_info, 1, wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_info_team->Add(sizer_info, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_map_label->Add(label_map, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_author->Add(label_author_prefix, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_author->Add(label_author, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_map_label->Add(grid_sizer_author, 1, wxALIGN_CENTER_HORIZONTAL, 2);
    grid_sizer_map_label->AddGrowableRow(2);
    grid_sizer_map_label->AddGrowableCol(0);
    sizer_map_label->Add(grid_sizer_map_label, 1, wxALIGN_CENTER_VERTICAL, 0);
    grid_sizer_info_team->Add(sizer_map_label, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_info_team->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_info_team, 1, wxEXPAND, 0);
    grid_sizer_update->Add(button_update, 0, wxALL|wxEXPAND, 4);
    grid_sizer_update->Add(checkbox_update, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_update->Add(checkbox_update_end, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_update->Add(1, 1, 0, 0, 0);
    grid_sizer_update->Add(checkbox_map, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_update->AddGrowableCol(3);
    sizer_update->Add(grid_sizer_update, 1, wxEXPAND, 4);
    grid_sizer_main->Add(sizer_update, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_main->Add(static_line, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 4);
    grid_sizer_button->Add(button_close, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_main->Add(grid_sizer_button, 1, wxTOP|wxBOTTOM|wxALIGN_RIGHT, 4);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    m_gridSizerMain=grid_sizer_main;
    m_gridSizerList=grid_sizer_list_map;
    m_gridSizerInfo=grid_sizer_info_team;
    m_sizerMap=sizer_map;
    m_sizerMapLabel=sizer_map_label;

    Layout();
    grid_sizer_main->SetSizeHints(this);
    CentreOnScreen();
}

void CslDlgExtended::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        Hide();
        return;
    }
    event.Skip();
}

void CslDlgExtended::OnSize(wxSizeEvent& event)
{
    ListAdjustSize(list_ctrl_players->GetClientSize());

    event.Skip();
}

void CslDlgExtended::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslDlgExtended::OnTimer(wxTimerEvent& event)
{
    if (!IsShown())
        return;

    CslPlayerStats *stats=m_info->m_playerStats;
    wxUint32 now=wxDateTime::Now().GetTicks();
    wxString label=_("&Update");

    if (!button_update->IsEnabled() && m_info->m_players>0)
    {
        wxInt32 diff=CSL_EXT_REFRESH_INTERVAL-(now-stats->m_lastRefresh);
        if (diff>0)
            label+=wxString::Format(wxT(" (%d)"),diff);
    }

    if (stats->m_ids.length())
    {
        wxUint32 delay=m_info->m_ping*m_info->m_playersMax;
        if ((delay > (wxUint32)CSL_EXT_REFRESH_INTERVAL*1000) ||
            ((now-stats->m_lastResponse)*1000 < delay))
            return;

        loopv(stats->m_ids)
        {
            wxInt32 id=stats->m_ids[i];
            QueryInfo(id);
            LOG_DEBUG("Resend %d\n",id);
        }
        stats->m_lastResponse=now;
        return;
    }

    if (!button_update->IsEnabled() && m_info->m_players>0)
        button_update->SetLabel(label);

    if (m_info->m_players && stats->m_lastRefresh+CSL_EXT_REFRESH_INTERVAL<=now)
    {
        if (checkbox_update->GetValue())
            QueryInfo();
        else if (!button_update->IsEnabled())
            button_update->Enable();
    }
    else if (!m_info->m_players && list_ctrl_players->GetItemCount())
        list_ctrl_players->DeleteAllItems();
}

void CslDlgExtended::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxID_CLOSE:
            Close();
            break;

        case CHECK_UPDATE:
            checkbox_update_end->Enable(event.IsChecked());
            break;

        case CHECK_MAP:
            if (panel_map->IsOk())
                ShowPanelMap(event.IsChecked());
            break;

        case BUTTON_REFRESH:
            QueryInfo();
            break;
    }

    event.Skip();
}

void CslDlgExtended::OnPong(wxCommandEvent& event)
{
    CslPongPacket *pp=(CslPongPacket*)event.GetClientData();
    wxASSERT(pp);
    if (!pp)
        return;

    CslServerInfo *info=pp->m_info;
    if (m_info!=info || !m_info || !info)
    {
        delete pp;
        return;
    }

    switch (pp->m_type)
    {
        case CSL_PONG_TYPE_PLAYERSTATS:
            SetPlayerData();
            break;

        case CSL_PONG_TYPE_TEAMSTATS:
            SetTeamData();
            break;

        default:
            break;
    }

    delete pp;
}

void CslDlgExtended::UpdateMap()
{
    if (m_info->m_map.IsEmpty())
    {
        m_mapInfo.Reset();
        panel_map->SetOk(false);
        ShowPanelMap(false);

        label_map->SetLabel(_("no map"));
        label_author->SetLabel(wxEmptyString);
        label_author_prefix->SetLabel(wxEmptyString);

        return;
    }

    if (m_mapInfo.m_mapName!=m_info->m_map)
    {
        if (m_mapInfo.LoadMapData(m_info->m_map,GetGameStr(m_info->m_type),m_info->m_protocol))
        {
            if (m_mapInfo.m_mapNameFull.IsEmpty())
                label_map->SetLabel(m_mapInfo.m_mapName);
            else
                label_map->SetLabel(m_mapInfo.m_mapNameFull);
            if (m_mapInfo.m_author.IsEmpty())
            {
                label_author->SetLabel(wxEmptyString);
                label_author_prefix->SetLabel(wxEmptyString);
            }
            else
            {
                label_author_prefix->SetLabel(_("by"));
                label_author->SetLabel(m_mapInfo.m_author);
            }

            panel_map->SetMap(m_mapInfo.m_bitmap,checkbox_map->IsChecked());
            ShowPanelMap(checkbox_map->IsChecked());
        }
        else
        {
            panel_map->SetOk(false);
            ShowPanelMap(false);

            label_map->SetLabel(m_info->m_map);
            label_author->SetLabel(wxEmptyString);
            label_author_prefix->SetLabel(wxEmptyString);
        }
    }
}

void CslDlgExtended::ClearTeamScoreLabel(const wxUint32 start,const wxUint32 end)
{
    wxUint32 i=start;
    wxUint32 j=end;

    for (;i<j;i++)
    {
        m_teamLabel.Item(i)->SetLabel(wxT(""));
        m_teamLabel.Item(i)->SetFont(m_labelFont);
    }
}
void CslDlgExtended::SetPlayerData()
{
    wxInt32 c;
    wxString s;
    wxListItem item;
    CslPlayerStats *stats=m_info->m_playerStats;
    CslPlayerStatsData *data=NULL;

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

    loopv(stats->m_stats)
    {
        data=stats->m_stats[i];
        if (!data->m_ok)
            break;
        if (i<list_ctrl_players->GetItemCount())
            continue;

        item.SetId(i);
        list_ctrl_players->InsertItem(item);
        list_ctrl_players->SetItemData(i,(long)data);

        if (data->m_player.IsEmpty())
            s=_("- connecting -");
        else
            s=data->m_player;
        list_ctrl_players->SetItem(i,0,s);

        if (data->m_state==CSL_PLAYER_STATE_SPECTATOR)
            s=_("Spectator");
        else
            s=data->m_team;
        list_ctrl_players->SetItem(i,1,s);

        if (data->m_frags<0)
            s=_("no data");
        else
            s=wxString::Format(wxT("%d"),data->m_frags);
        list_ctrl_players->SetItem(i,2,s);

        if (data->m_deaths<0)
            s=_("no data");
        else
            s=wxString::Format(wxT("%d"),data->m_deaths);
        list_ctrl_players->SetItem(i,3,s);

        if (data->m_teamkills<0)
            s=_("no data");
        else
            s=wxString::Format(wxT("%d"),data->m_teamkills);
        list_ctrl_players->SetItem(i,4,s);

        if (data->m_state==CSL_PLAYER_STATE_UNKNOWN)
            s=_("no data");
        else if (data->m_state==CSL_PLAYER_STATE_DEAD)
            s=_("dead");
        else if (data->m_state==CSL_PLAYER_STATE_EDITING)
            s=_("editing");
        else if (data->m_state)
            s=wxT("0");
        else
            s=wxString::Format(wxT("%d"),data->m_health);
        list_ctrl_players->SetItem(i,5,s);

        if (data->m_state==CSL_PLAYER_STATE_UNKNOWN)
            s=_("no data");
        else if (data->m_state || data->m_armour<0)
            s=wxT("0");
        else
            s=wxString::Format(wxT("%d"),data->m_armour);
        list_ctrl_players->SetItem(i,6,s);

        s=CslGame::GetWeaponName(m_info->m_type,data->m_weapon);
        list_ctrl_players->SetItem(i,7,s);

        wxColour colour=*wxBLACK;
        if (data->m_priv==CSL_PLAYER_PRIV_MASTER)
            list_ctrl_players->SetItemBackgroundColour(item,wxColour(64,255,128));
        else if (data->m_priv==CSL_PLAYER_PRIV_ADMIN)
            list_ctrl_players->SetItemBackgroundColour(item,wxColour(255,128,0));
        else if (data->m_state==CSL_PLAYER_STATE_SPECTATOR)
            list_ctrl_players->SetItemBackgroundColour(item,wxColour(192,192,192));
        /*if (data->m_priv==CSL_PLAYER_PRIV_MASTER)
            i=CSL_LIST_IMG_GREEN;
        else if (data->m_priv==CSL_PLAYER_PRIV_ADMIN)
            i=CSL_LIST_IMG_ORANGE;
        else if (data->m_state==CSL_PLAYER_STATE_SPECTATOR)
            i=CSL_LIST_IMG_GREY;
        else
            i=CSL_LIST_IMG_TRANS;*/

        const char *country;
        wxInt32 flag=CSL_LIST_IMG_UNKNOWN;

        if (data->m_ip && CslGeoIP::IsOk())
        {
            country=CslGeoIP::GetCountryCodeByIPnum(data->m_ip);
            if (country)
            {
                for (c=sizeof(codes)/sizeof(codes[0])-1;c>=0;c--)
                {
                    if (!strcasecmp(country,codes[c]))
                    {
                        flag+=++c;
                        break;
                    }
                }
            }
        }

        list_ctrl_players->SetItemImage(item,flag);
    }


    stats->m_lastResponse=wxDateTime::Now().GetTicks();

    label_records->SetLabel(wxString::Format(_("%d players (%d left)"),
                            list_ctrl_players->GetItemCount(),stats->m_ids.length()));

    ListSort(-1);
}

void CslDlgExtended::SetTeamData()
{
    CslTeamStats *stats;
    CslTeamStatsData *data;
    wxSize size;
    wxString s;
    bool captureAC,captureSB;
    wxInt32 h=-1,hs=0;
    wxInt32 lu=0,lc=m_teamLabel.GetCount();

    wxASSERT(m_info && m_info->m_teamStats);
    if (!m_info || !m_info->m_teamStats)
        return;

    stats=m_info->m_teamStats;

    UpdateMap();
    m_mapInfo.ResetBasesColour();

    captureAC=m_info->m_type==CSL_GAME_AC && m_info->m_isCapture;
    captureSB=m_info->m_type==CSL_GAME_SB && m_info->m_isCapture;

    if (stats->m_teamplay)
    {
        loopv(stats->m_stats)
        {
            data=stats->m_stats[i];

            // TODO remove i==c and dynamically add labels
            if (!data->m_ok || i==lc)
                break;

            if (data->m_score>hs)
            {
                hs=data->m_score;
                h=i;
            }

            // colour bases
            if (captureSB && m_mapInfo.m_basesOk)
            {
                wxInt32 baseId;
                wxInt32 baseCount=m_mapInfo.m_bases.GetCount();
                loopvj(data->m_bases)
                {
                    baseId=data->m_bases[j];
                    LOG_DEBUG("team: %s base:%d\n",U2A(data->m_team),baseId);

                    if (baseId<baseCount)
                        m_mapInfo.m_bases.Item(baseId)->m_colour=team_colours[i];
                }
            }

            s=data->m_team;
            s+=wxT(": ");
            s+=wxString::Format(wxT("%d"),data->m_score);

            if (captureAC)
            {
                wxInt32 flagscore=data->m_score2;
                if (flagscore<0)
                    flagscore=0;
                s+=wxString::Format(wxT(" / %d"),flagscore);
                sizer_team_score_staticbox->SetLabel(_("Team score")+wxString(wxT(" / "))+_("Flag score"));
            }
            else
                sizer_team_score_staticbox->SetLabel(_("Team score"));

            m_teamLabel.Item(i)->SetFont(m_labelFont);
            m_teamLabel.Item(i)->SetLabel(s);
            m_teamLabel.Item(i)->SetForegroundColour(team_colours[i]);

            size=m_teamLabel.Item(i)->GetBestSize();
            if (i%2==0)
                size.x+=40;
            else
                size.x+=6;
            m_teamLabel.Item(i)->SetMinSize(size);

            lu++;
        }

        // TODO remove h<c and dynamically add labels
        if (h>-1 && h<lc)
        {
            data=stats->m_stats[h];

            if (!captureAC)
            {
                wxFont font=m_labelFont;
                font.SetWeight(wxFONTWEIGHT_BOLD);
                m_teamLabel.Item(h)->SetFont(font);
            }

            if (data->m_score==10000 || stats->m_remain==0)
            {
                if (!captureAC)
                {
                    s=data->m_team;
                    s+=wxT(": ");
                    if (data->m_score==10000)
                        s+=_("WINNER");
                    else
                        s+=wxString::Format(wxT("%d - "),data->m_score)+_("WINNER");

                    m_teamLabel.Item(h)->SetLabel(s);
                    m_teamLabel.Item(h)->SetMinSize(m_teamLabel.Item(h)->GetBestSize());
                }

                if (checkbox_update_end->IsChecked())
                    checkbox_update->SetValue(false);
            }
        }
    }
    else
    {
        label_team1->SetFont(m_labelFont);
        label_team1->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
        if (!m_info->m_players)
            label_team1->SetLabel(_("No teams."));
        else
            label_team1->SetLabel(_("Not a team game."));
        label_team1->SetMinSize(label_team1->GetBestSize());
    }

    panel_map->UpdateBases(m_mapInfo.m_bases,captureSB);
    ClearTeamScoreLabel(lu ? lu : 1,lc);

    s=FormatSeconds(stats->m_remain>-1 ? stats->m_remain*60 : 0,true,true);
    if (s.IsEmpty())
        s=_("Time is up");
    label_remaining->SetLabel(s);
    label_mode->SetLabel(m_info->m_gameMode);

    RecalcMinSize(true);
}

void CslDlgExtended::QueryInfo(wxInt32 pid)
{
    if (m_info->m_players==0)
        return;

    if (pid==-1)
    {
        wxUint32 now=wxDateTime::Now().GetTicks();

        if (m_info->m_playerStats->m_lastRefresh+CSL_EXT_REFRESH_INTERVAL<=now)
        {
            list_ctrl_players->DeleteAllItems();
            label_records->SetLabel(_("Waiting for data..."));
            button_update->Enable(false);

            m_engine->PingExPlayerInfo(m_info);
            m_engine->PingExTeamInfo(m_info);

            m_info->m_playerStats->m_lastRefresh=now;
        }
    }
    else
        m_engine->PingExPlayerInfo(m_info,pid);
}

void CslDlgExtended::RecalcMinSize(bool reLayout,wxInt32 decWidth)
{
    bool height=false, width=false;

    if (reLayout)
        m_gridSizerMain->Layout();

    wxSize best=GetBestSize();
    wxSize size=GetSize();

    if (decWidth!=-1)
    {
        width=true;
        size.x+=decWidth;
    }

    if (best.x>size.x)
    {
        width=true;
        size.x=best.x;
    }
    if (best.y>size.y)
    {
        height=true;
        size.y=best.y;
    }

    if (width || height)
    {
        SetMinSize(wxSize(width ? size.x : -1,height ? size.y : -1));
        SetSize(width ? size.x : -1,height ? size.y : -1);
    }
    SetSizeHints(best);

    if (reLayout)
        ListAdjustSize(list_ctrl_players->GetClientSize());

#ifdef __WXMSW__
    checkbox_update->Refresh();
    checkbox_update_end->Refresh();
    checkbox_map->Refresh();
#endif
}

void CslDlgExtended::ListAdjustSize(const wxSize& size)
{
    if (list_ctrl_players->GetColumnCount()<6)
        return;

    wxInt32 w=size.x-8;

    list_ctrl_players->SetColumnWidth(0,(wxInt32)(w*0.23f));
    list_ctrl_players->SetColumnWidth(1,(wxInt32)(w*0.11f));
    list_ctrl_players->SetColumnWidth(2,(wxInt32)(w*0.11f));
    list_ctrl_players->SetColumnWidth(3,(wxInt32)(w*0.11f));
    list_ctrl_players->SetColumnWidth(4,(wxInt32)(w*0.11f));
    list_ctrl_players->SetColumnWidth(5,(wxInt32)(w*0.11f));
    list_ctrl_players->SetColumnWidth(6,(wxInt32)(w*0.11f));
    list_ctrl_players->SetColumnWidth(7,(wxInt32)(w*0.11f));
}

void CslDlgExtended::ListSort(wxInt32 column)
{
    wxListItem item;
    wxInt32 img;

    if (column==-1)
        column=m_sortHelper.m_sortType;
    else
    {
        item.SetMask(wxLIST_MASK_IMAGE);
        list_ctrl_players->GetColumn(column,item);

        if (item.GetImage()==-1 || item.GetImage()==CSL_LIST_IMG_SORT_DSC)
        {
            img=CSL_LIST_IMG_SORT_ASC;
            m_sortHelper.m_sortMode=CSL_SORT_ASC;
        }
        else
        {
            img=CSL_LIST_IMG_SORT_DSC;
            m_sortHelper.m_sortMode=CSL_SORT_DSC;
        }

        item.Clear();
        item.SetImage(-1);
        list_ctrl_players->SetColumn(m_sortHelper.m_sortType,item);

        item.SetImage(img);
        list_ctrl_players->SetColumn(column,item);

        m_sortHelper.m_sortType=column;
    }

    if (list_ctrl_players->GetItemCount()>0)
        list_ctrl_players->SortItems(ListSortCompareFunc,(long)&m_sortHelper);
}

void CslDlgExtended::ToggleSortArrow()
{
    wxListItem item;
    wxInt32 img=-1;

    if (m_sortHelper.m_sortMode==CSL_SORT_ASC)
        img=CSL_LIST_IMG_SORT_ASC;
    else
        img=CSL_LIST_IMG_SORT_DSC;

    item.SetImage(img);
    list_ctrl_players->SetColumn(m_sortHelper.m_sortType,item);
}

void CslDlgExtended::ShowPanelMap(const bool show)
{
    wxInt32 width;

    if (m_mapInfo.m_bitmap.IsOk())
    {
        width=m_mapInfo.m_bitmap.GetWidth();
#ifdef __WXMAC__
        width+=30;
#else
        width+=16;
#endif
    }
    else
        width=-1;

    if (show)
        m_sizerMapLabel->SetMinSize(width-8,-1);

    if (show && !m_gridSizerList->GetItem(m_sizerMap))
    {
        Freeze();

        m_gridSizerList->Add(m_sizerMap,1,wxALL|wxEXPAND,3);
        m_gridSizerList->Show(m_sizerMap);
    }
    else if (!show && m_gridSizerList->GetItem(m_sizerMap))
    {
        m_gridSizerList->Hide(m_sizerMap);
        m_gridSizerList->Detach(m_sizerMap);

        m_sizerMapLabel->SetMinSize(m_sizerMapLabel->GetStaticBox()->GetBestSize());
    }
    else
        return;

    RecalcMinSize(true,show ? width : width/-1);

    if (show)
        Thaw();
}

void CslDlgExtended::ListInit(CslEngine *engine)
{
    wxListItem item;

    m_engine=engine;

    list_ctrl_players->SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    list_ctrl_players->InsertColumn(0,item);
    list_ctrl_players->SetColumn(0,item);

    item.SetText(_("Team"));
    list_ctrl_players->InsertColumn(1,item);
    list_ctrl_players->SetColumn(1,item);

    item.SetText(_("Frags"));
    list_ctrl_players->InsertColumn(2,item);
    list_ctrl_players->SetColumn(2,item);

    item.SetText(_("Deaths"));
    list_ctrl_players->InsertColumn(3,item);
    list_ctrl_players->SetColumn(3,item);

    item.SetText(_("Teamkills"));
    list_ctrl_players->InsertColumn(4,item);
    list_ctrl_players->SetColumn(4,item);

    item.SetText(_("Health"));
    list_ctrl_players->InsertColumn(5,item);
    list_ctrl_players->SetColumn(5,item);

    item.SetText(_("Armour"));
    list_ctrl_players->InsertColumn(6,item);
    list_ctrl_players->SetColumn(6,item);

    item.SetText(_("Weapon"));
    list_ctrl_players->InsertColumn(7,item);
    list_ctrl_players->SetColumn(7,item);

    // assertion on __WXMAC__
    //ListAdjustSize(GetClientSize());

    m_sortHelper.Init(CSL_SORT_DSC,SORT_FRAGS);
    ToggleSortArrow();
}

void CslDlgExtended::DoShow(CslServerInfo *info)
{
    wxString s;

    if (!info || !m_engine)
    {
        wxASSERT(info && m_engine);
        return;
    }

    if (m_info!=info)
    {
        list_ctrl_players->DeleteAllItems();
        label_records->SetLabel(wxString::Format(_("%d players (%d left)"),0,0));
    }

    m_info=info;

    SetTeamData();

    button_update->SetLabel(_("&Update"));
    button_update->Enable(false);
    label_server->SetLabel(m_info->GetBestDescription());
    label_mode->SetLabel(m_info->m_gameMode);
    if (m_info->m_timeRemain>0)
        s=FormatSeconds(m_info->m_timeRemain*60,true,true);
    else
        s=_("Time is up");
    label_remaining->SetLabel(s);

    s=wxString(_("CSL - Extended info"))+wxString(wxT(": "))+m_info->GetBestDescription();
    SetTitle(s);

    QueryInfo();

    if (IsShown())
        return;

    Show();
}

int wxCALLBACK CslDlgExtended::ListSortCompareFunc(long item1,long item2,long data)
{
    CslPlayerStatsData *stats1=(CslPlayerStatsData*)item1;
    CslPlayerStatsData *stats2=(CslPlayerStatsData*)item2;

    wxInt32 type;
    wxInt32 sortMode=((CslListSortHelper*)data)->m_sortMode;
    wxInt32 sortType=((CslListSortHelper*)data)->m_sortType;
    wxInt32 vi1=0,vi2=0;
    wxUint32 vui1=0,vui2=0;
    wxString vs1=wxEmptyString,vs2=wxEmptyString;

    switch (sortType)
    {
        case SORT_PLAYER:
            type=CSL_LIST_SORT_STRING;
            vs1=stats1->m_player;
            vs2=stats2->m_player;
            break;

        case SORT_TEAM:
            type=CSL_LIST_SORT_STRING;
            vs1=stats1->m_team;
            vs2=stats2->m_team;
            break;

        case SORT_FRAGS:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_frags;
            vi2=stats2->m_frags;
            break;

        case SORT_DEATHS:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_deaths;
            vi2=stats2->m_deaths;
            break;

        case SORT_TEAMKILLS:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_teamkills;
            vi2=stats2->m_teamkills;
            break;

        case SORT_HEALTH:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_health;
            vi2=stats2->m_health;
            break;

        case SORT_ARMOUR:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_armour;
            vi2=stats2->m_armour;
            break;

        case SORT_WEAPON:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_weapon;
            vi2=stats2->m_weapon;
            break;

        default:
            return 0;
    }

    if (type==CSL_LIST_SORT_INT)
    {
        if (vi1==vi2)
            return 0;
        if (vi1<vi2)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }
    else if (type==CSL_LIST_SORT_UINT)
    {
        if (vui1==vui2)
            return 0;
        if (vui1<vui2)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }
    else if (type==CSL_LIST_SORT_STRING)
    {
        if (vs1==vs2)
            return 0;
        if (vs1.CmpNoCase(vs2)<0)
            return sortMode==CSL_SORT_ASC ? -1 : 1;
        else
            return sortMode==CSL_SORT_ASC ? 1 : -1;
    }

    return 0;
}
