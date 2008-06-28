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

#include <wx/wupdlock.h>
#include "CslDlgExtended.h"
#include "CslSettings.h"
#include "CslMenu.h"

BEGIN_EVENT_TABLE(CslDlgExtended, wxDialog)
    EVT_CLOSE(CslDlgExtended::OnClose)
    EVT_SIZE(CslDlgExtended::OnSize)
    EVT_BUTTON(wxID_ANY,CslDlgExtended::OnCommandEvent)
    EVT_CHECKBOX(wxID_ANY,CslDlgExtended::OnCommandEvent)
    EVT_MENU(wxID_ANY,CslDlgExtended::OnMenu)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslDlgExtended::OnItemActivated)
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
    m_info=NULL;
    m_gridSizerMain=m_gridSizerList=m_gridSizerInfo=NULL;
    m_sizerMap=m_sizerMapLabel=NULL;

    // begin wxGlade: CslDlgExtended::CslDlgExtended
    sizer_team_score_staticbox = new wxStaticBox(this, -1, _("Team score"));
    sizer_info_staticbox = new wxStaticBox(this, -1, _("Game information"));
    sizer_map_label_staticbox = new wxStaticBox(this, -1, _("Map information"));
    sizer_map_staticbox = new wxStaticBox(this, -1, _("Map"));
    list_ctrl_players = new CslListCtrlPlayer(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
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
    checkbox_update_end = new wxCheckBox(this, wxID_ANY, _("Stop auto update when map &ends"));
    checkbox_map = new wxCheckBox(this, CHECK_MAP, _("&Show map if possible"));
    static_line = new wxStaticLine(this, wxID_ANY);
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade
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

    list_ctrl_players->ListInit(MENU_SERVER_EXTENDED_DEFAULT-MENU_SERVER_EXTENDED_MICRO);
#ifdef __WXMSW__
    list_ctrl_players->SetMinSize(wxSize(520,270));
#elif __WXGTK__
    list_ctrl_players->SetMinSize(wxSize(580,320));
#elif __WXMAC__
    list_ctrl_players->SetMinSize(wxSize(580,330));
#endif
}

void CslDlgExtended::do_layout()
{
    // begin wxGlade: CslDlgExtended::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(5, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_checkboix = new wxFlexGridSizer(1, 2, 0, 0);
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
    wxStaticText* label_records_static = new wxStaticText(this, wxID_ANY, _("Players :"));
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
    grid_sizer_checkboix->Add(checkbox_update_end, 0, wxALL, 4);
    grid_sizer_checkboix->Add(checkbox_map, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_checkboix->AddGrowableCol(0);
    grid_sizer_main->Add(grid_sizer_checkboix, 1, wxEXPAND, 0);
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
    list_ctrl_players->ListAdjustSize();
    event.Skip();
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
        default:
            break;
    }

    event.Skip();
}

void CslDlgExtended::OnMenu(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MENU_SERVER_CONNECT:
        case MENU_SERVER_CONNECT_PW:
            wxPostEvent(m_parent,event);
        default:
            return;
    }
}

void CslDlgExtended::OnItemActivated(wxListEvent& event)
{
    wxPostEvent(m_parent,event);
}

void CslDlgExtended::UpdateMap()
{
    if (m_info->Map.IsEmpty())
    {
        m_mapInfo.Reset();
        panel_map->SetOk(false);
        ShowPanelMap(false);

        label_map->SetLabel(_("no map"));
        label_author->SetLabel(wxEmptyString);
        label_author_prefix->SetLabel(wxEmptyString);

        return;
    }

    if (m_mapInfo.m_mapName!=m_info->Map)
    {
        if (m_mapInfo.LoadMapData(m_info->Map,m_info->GetGame().GetName(),m_info->Protocol))
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

            label_map->SetLabel(m_info->Map);
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
void CslDlgExtended::UpdatePlayerData()
{
    list_ctrl_players->ListUpdatePlayerData(m_info->GetGame(),m_info->PlayerStats);
    label_records->SetLabel(wxString::Format(_("%d players"),list_ctrl_players->GetItemCount()));
}

void CslDlgExtended::UpdateTeamData()
{
    if (!m_info)
        return;

    CslTeamStatsData *data;
    wxSize size;
    wxString s;
    wxInt32 h=-1,hs=0;
    wxInt32 lu=0,lc=m_teamLabel.GetCount();

    CslTeamStats& stats=m_info->TeamStats;

#ifdef __WXMSW__
    //fixes flicker of teamscore labels
    wxWindowUpdateLocker lock(this);
#endif

    UpdateMap();
    m_mapInfo.ResetBasesColour();

    if (stats.m_teamplay)
    {
        loopv(stats.m_stats)
        {
            data=stats.m_stats[i];

            // TODO remove i==c and dynamically add labels
            if (!data->Ok || i==lc)
                break;

            if (data->Score>hs)
            {
                hs=data->Score;
                h=i;
            }

            // colour bases
            if (m_info->ModeHasBases && m_mapInfo.m_basesOk)
            {
                wxInt32 baseId;
                wxInt32 baseCount=m_mapInfo.m_bases.GetCount();
                loopvj(data->Bases)
                {
                    baseId=data->Bases[j];

                    if (baseId<baseCount)
                        m_mapInfo.m_bases.Item(baseId)->m_colour=team_colours[i];
                }
            }

            s=data->Name;
            s+=wxT(": ");
            s+=wxString::Format(wxT("%d"),data->Score);

            if (CSL_SCORE_IS_VALID(data->Score2))
            {
                wxInt32 score2=data->Score2;
                if (score2<0)
                    score2=0;
                s+=wxString::Format(wxT(" / %d"),score2);
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
            data=stats.m_stats[h];

            if (!m_info->IsCapture)
            {
                wxFont font=m_labelFont;
                font.SetWeight(wxFONTWEIGHT_BOLD);
                m_teamLabel.Item(h)->SetFont(font);
            }

            if (data->Score==10000 || stats.m_remain==0)
            {
                if (!m_info->IsCapture)
                {
                    s=data->Name;
                    s+=wxT(": ");
                    if (data->Score==10000)
                        s+=_("WINNER");
                    else
                        s+=wxString::Format(wxT("%d - "),data->Score)+_("WINNER");

                    m_teamLabel.Item(h)->SetLabel(s);
                    m_teamLabel.Item(h)->SetMinSize(m_teamLabel.Item(h)->GetBestSize());
                }

                if (checkbox_update_end->IsChecked())
                {
                }
            }
        }
    }
    else
    {
        label_team1->SetFont(m_labelFont);
        label_team1->SetForegroundColour(SYSCOLOUR(wxSYS_COLOUR_WINDOWTEXT));
        if (!m_info->Players)
            label_team1->SetLabel(_("No teams."));
        else
            label_team1->SetLabel(_("Not a team game."));
        label_team1->SetMinSize(label_team1->GetBestSize());
    }

    panel_map->UpdateBases(m_mapInfo.m_bases,m_info->ModeHasBases);
    ClearTeamScoreLabel(lu ? lu : 1,lc);

    s=FormatSeconds(stats.m_remain>-1 ? stats.m_remain*60 : 0,true,true);
    if (s.IsEmpty())
        s=_("Time is up");
    label_remaining->SetLabel(s);
    label_mode->SetLabel(m_info->GameMode);

    RecalcMinSize(true);
}

void CslDlgExtended::RecalcMinSize(bool reLayout,wxInt32 decWidth)
{
    bool height=false,width=false;

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
        list_ctrl_players->ListAdjustSize();

#ifdef __WXMSW__
    checkbox_update_end->Refresh();
    checkbox_map->Refresh();
#endif
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
#ifdef __WXMAC__
        m_sizerMapLabel->SetMinSize(wxSize(-1,-1));
#else
        m_sizerMapLabel->SetMinSize(m_sizerMapLabel->GetStaticBox()->GetBestSize());
#endif
    }
    else
        return;

    RecalcMinSize(true,show ? width : width/-1);

    if (show)
        Thaw();
}

void CslDlgExtended::DoShow(CslServerInfo *info)
{
    wxString s;

    if (m_info!=info)
    {
        list_ctrl_players->DeleteAllItems();
        label_records->SetLabel(wxString::Format(_("%d players"),0));
    }

    m_info=info;
    list_ctrl_players->SetInfo(info);

    UpdatePlayerData();
    UpdateTeamData();

    label_server->SetLabel(m_info->GetBestDescription());
    label_mode->SetLabel(m_info->GameMode);
    if (m_info->TimeRemain>0)
        s=FormatSeconds(m_info->TimeRemain*60,true,true);
    else
        s=_("Time is up");
    label_remaining->SetLabel(s);

    s=wxString(_("CSL - Extended info"))+wxString(wxT(": "))+m_info->GetBestDescription();
    SetTitle(s);

    if (IsShown())
        return;

    Show();
}
