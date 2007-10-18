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

#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include "CslDlgExtended.h"
#include "CslSettings.h"
#ifndef _MSC_VER
#include "img/sortasc_16.xpm"
#include "img/sortdsc_16.xpm"
#include "img/green_list_16.xpm"
#include "img/orange_list_16.xpm"
#include "img/grey_list_16.xpm"
#include "img/trans_list_16.xpm"
#endif

BEGIN_EVENT_TABLE(CslPanelMap, wxPanel)
    EVT_PAINT(CslPanelMap::OnPaint)
    EVT_SIZE(CslPanelMap::OnSize)
END_EVENT_TABLE()

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
    CSL_LIST_IMG_GREEN,
    CSL_LIST_IMG_ORANGE,
    CSL_LIST_IMG_GREY,
    CSL_LIST_IMG_TRANS
};

enum
{
    SORT_PLAYER = 0, SORT_TEAM,
    SORT_FRAGS, SORT_DEATHS, SORT_TEAMKILLS,
    SORT_HEALTH, SORT_ARMOUR, SORT_WEAPON
};

enum { BUTTON_REFRESH = wxID_HIGHEST +1, CHECK_UPDATE, CHECK_MAP };

#define CSL_EXT_REFRESH_INTERVAL  g_cslSettings->m_updateInterval/1000


bool CslMapInfo::LoadMapData(const wxString& mapName,const wxString& gameName,
                             const wxUint32 protVersion)
{
    wxUint32 version=protVersion;

    wxString path=wxString(wxT(DATADIR))+PATHDIV+wxT("maps")+PATHDIV+gameName+PATHDIV;
#ifdef __WXGTK__
    if (!::wxDirExists(path))
        path=::wxPathOnly(wxTheApp->argv[0])+wxT("/data/maps/")+gameName+PATHDIV;
#endif

    Reset();

    if (::wxFileExists(path+mapName+wxT(".cfg")))
    {
        wxFileInputStream stream(path+mapName+wxT(".cfg"));
        if (!stream.IsOk())
            return false;

        CslBaseInfo *base;
        long int val;
        wxInt32 x,y;
        wxUint32 i=0;

        wxFileConfig config(stream);

        config.SetPath(wxT("/LastVersion"));
        if (config.Read(wxT("LastVersion"),&val))
        {
            if (val!=(long int)version)
                version=val;

            config.SetPath(wxString::Format(wxT("/%d/Mapname"),version));
            config.Read(wxT("Mapname"),&m_mapNameFull);
            config.SetPath(wxString::Format(wxT("/%d/Author"),version));
            config.Read(wxT("Author"),&m_author);

            path+=wxString::Format(wxT("%d/"),version)+mapName+wxT(".png");
            if (::wxFileExists(path))
            {
                if (!m_bitmap.LoadFile(path,wxBITMAP_TYPE_PNG))
                    return false;
            }
            else
                return false;
        }
        else
            return false;

        while (true)
        {
            config.SetPath(wxString::Format(wxT("/%d/Base%d"),version,i));
            if (!config.Read(wxT("x"),&val))
                break;
            x=val;
            if (!config.Read(wxT("y"),&val))
                break;
            y=val;

            base=new CslBaseInfo(x,y);
            m_bases.Add(base);

            i++;
        }

        if (m_bases.GetCount())
            m_basesOk=true;

        return true;
    }

    return false;
}


void CslPanelMap::OnPaint(wxPaintEvent& event)
{
    wxUint32 i;

    wxPaintDC dc(this);
    PrepareDC(dc);

    if (m_ok)
    {
        wxPoint origin=GetBitmapOrigin();

        m_memDC.SelectObject(m_bitmap);
        dc.Blit(origin.x,origin.y,m_bitmap.GetWidth(),m_bitmap.GetHeight(),&m_memDC,0,0);

        for (i=0;i<m_bases.GetCount();i++)
        {
            CslBaseInfo *base=m_bases.Item(i);
            //dc.SetPen(wxPen(base->m_colour));
            dc.SetPen(wxPen(*wxWHITE));
            dc.SetBrush(wxBrush(base->m_colour));
            dc.DrawCircle(origin.x+base->m_point.x,origin.y+base->m_point.y,5);
        }
    }

    event.Skip();
}

void CslPanelMap::UpdateBases(const t_aBaseInfo& bases,const bool hasBases)
{
    wxUint32 i;
    wxRect rect;
    wxPoint point;
    CslBaseInfo *base;

    WX_CLEAR_ARRAY(m_bases);

    if (hasBases)
        for (i=0;i<bases.GetCount();i++)
        {
            base=new CslBaseInfo(*bases.Item(i));
            m_bases.Add(base);
        }

    if (!IsShown())
        return;

    Refresh();
    /*for (i=0;i<bases.GetCount();i++)
    {
        wxPoint point=bases.Item(i)->m_point;
        wxRect rect(point.x-5,point.y-5,10,10);
        RefreshRect(rect);
    }*/
}

CslDlgExtended::CslDlgExtended(wxWindow* parent,int id,const wxString& title,
                               const wxPoint& pos, const wxSize& size, long style):
        wxDialog(parent, id, title, pos, size, style),
        m_engine(NULL),m_info(NULL)
{
    // begin wxGlade: CslDlgExtended::CslDlgExtended
    sizer_team_score_staticbox = new wxStaticBox(this, -1, _("Team score"));
    sizer_info_staticbox = new wxStaticBox(this, -1, _("Game info"));
    sizer_check_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    sizer_map_label_staticbox = new wxStaticBox(this, -1, _("Map info"));
    sizer_map_staticbox = new wxStaticBox(this, -1, _("Map info"));
    list_ctrl_extended = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
    panel_map = new CslPanelMap(this, wxID_ANY);
    label_team1 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team2 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team3 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team4 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team5 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_team6 = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_mode = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_remaining = new wxStaticText(this, wxID_ANY, wxEmptyString);
    label_records = new wxStaticText(this, wxID_ANY, wxEmptyString);
    checkbox_update = new wxCheckBox(this, CHECK_UPDATE, _("&Auto update"));
    checkbox_update_end = new wxCheckBox(this, wxID_ANY, _("Stop when game &ends"));
    checkbox_map = new wxCheckBox(this, CHECK_MAP, _("Show map"));
    label_map = new wxStaticText(this, wxID_ANY, _("label_map"));
    label_author_prefix = new wxStaticText(this, wxID_ANY, _("by"));
    label_author = new wxStaticText(this, wxID_ANY, _("label_author"));
    button_update = new wxButton(this, BUTTON_REFRESH, _("&Update"));
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade

    m_imageList.Create(16,16,true);
#ifndef _MSC_VER
    m_imageList.Add(wxBitmap(sortasc_16_xpm));
    m_imageList.Add(wxBitmap(sortdsc_16_xpm));
    m_imageList.Add(wxBitmap(green_list_16_xpm));
    m_imageList.Add(wxBitmap(orange_list_16_xpm));
    m_imageList.Add(wxBitmap(grey_list_16_xpm));
    m_imageList.Add(wxBitmap(trans_list_16_xpm));

#else
    m_imageList.Add(wxIcon(wxT("ICON_LIST_ASC"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_DSC"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_GREEN"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_ORANGE"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_GREY"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_TRANS"),wxBITMAP_TYPE_ICO_RESOURCE));
#endif

    m_labelFont=label_team1->GetFont();
}

CslDlgExtended::~CslDlgExtended()
{
    if (!panel_map->IsShown())
    {
        delete m_sizerMap;
        //delete m_sizerMapLabel;
    }
}

void CslDlgExtended::set_properties()
{
    // begin wxGlade: CslDlgExtended::set_properties
    SetTitle(_("CSL - Extended info"));
    label_team1->SetMinSize(wxSize(60, -1));
    label_team2->SetMinSize(wxSize(60, -1));
    label_mode->SetMinSize(wxSize(100, -1));
    checkbox_update_end->Enable(false);
    label_map->SetFont(wxFont(20, wxDECORATIVE, wxNORMAL, wxBOLD, 0, wxT("")));
    label_author->SetFont(wxFont(11, wxDECORATIVE, wxITALIC, wxBOLD, 0, wxT("")));
    button_close->SetDefault();
    // end wxGlade

    m_teamLabel.Add(label_team1);
    m_teamLabel.Add(label_team2);
    m_teamLabel.Add(label_team3);
    m_teamLabel.Add(label_team4);
    m_teamLabel.Add(label_team5);
    m_teamLabel.Add(label_team6);

    // wxMAC: have to set minsize of the listctrl to prevent
    //        hiding of the search panel while dragging splitter
    //list_ctrl_extended->SetMinSize(wxSize(0,60));

#ifdef __WXMSW__
    list_ctrl_extended->SetMinSize(wxSize(520,350));
#endif
#ifdef __WXGTK__
    list_ctrl_extended->SetMinSize(wxSize(580,350));
#endif
#ifdef __WXMAC__
    list_ctrl_extended->SetMinSize(wxSize(580,350));
#endif
//    list_ctrl_extended->SetMinSize(wxSize(-1,350));
}

void CslDlgExtended::do_layout()
{
    // begin wxGlade: CslDlgExtended::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(4, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_info_team = new wxFlexGridSizer(1, 4, 0, 0);
    wxStaticBoxSizer* sizer_map_label = new wxStaticBoxSizer(sizer_map_label_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_map_label = new wxFlexGridSizer(4, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_author = new wxFlexGridSizer(1, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_update = new wxFlexGridSizer(1, 1, 0, 0);
    wxStaticBoxSizer* sizer_check = new wxStaticBoxSizer(sizer_check_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_check = new wxFlexGridSizer(3, 1, 0, 0);
    wxStaticBoxSizer* sizer_info = new wxStaticBoxSizer(sizer_info_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_info = new wxFlexGridSizer(3, 2, 0, 0);
    wxStaticBoxSizer* sizer_team_score = new wxStaticBoxSizer(sizer_team_score_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_team_score = new wxFlexGridSizer(3, 2, 0, 0);
    wxFlexGridSizer* grid_sizer_list_map = new wxFlexGridSizer(1, 2, 0, 0);
    wxStaticBoxSizer* sizer_map = new wxStaticBoxSizer(sizer_map_staticbox, wxHORIZONTAL);
    wxFlexGridSizer* grid_sizer_map = new wxFlexGridSizer(3, 1, 0, 0);
    grid_sizer_list_map->Add(list_ctrl_extended, 1, wxALL|wxEXPAND, 4);
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
    sizer_team_score->Add(grid_sizer_team_score, 1, wxEXPAND, 0);
    grid_sizer_info_team->Add(sizer_team_score, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    wxStaticText* label_mode_static = new wxStaticText(this, wxID_ANY, _("Mode:"));
    grid_sizer_info->Add(label_mode_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_mode, 0, wxALL, 4);
    wxStaticText* label_remaining_static = new wxStaticText(this, wxID_ANY, _("Time remaining:"));
    grid_sizer_info->Add(label_remaining_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_remaining, 0, wxALL, 4);
    wxStaticText* label_records_static = new wxStaticText(this, wxID_ANY, _("Player records:"));
    grid_sizer_info->Add(label_records_static, 0, wxALL, 4);
    grid_sizer_info->Add(label_records, 0, wxALL, 4);
    sizer_info->Add(grid_sizer_info, 1, wxEXPAND, 0);
    grid_sizer_info_team->Add(sizer_info, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_check->Add(checkbox_update, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_check->Add(checkbox_update_end, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_check->Add(checkbox_map, 0, wxALL, 4);
    sizer_check->Add(grid_sizer_check, 1, wxEXPAND, 0);
    grid_sizer_update->Add(sizer_check, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_update->AddGrowableRow(0);
    grid_sizer_update->AddGrowableRow(1);
    grid_sizer_info_team->Add(grid_sizer_update, 1, wxEXPAND, 0);
    grid_sizer_map_label->Add(1, 1, 0, 0, 0);
    grid_sizer_map_label->Add(label_map, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
    grid_sizer_author->Add(label_author_prefix, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_author->Add(label_author, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_map_label->Add(grid_sizer_author, 1, wxALIGN_CENTER_HORIZONTAL, 0);
    grid_sizer_map_label->Add(1, 1, 0, 0, 0);
    grid_sizer_map_label->AddGrowableRow(0);
    grid_sizer_map_label->AddGrowableRow(3);
    grid_sizer_map_label->AddGrowableCol(0);
    sizer_map_label->Add(grid_sizer_map_label, 1, wxEXPAND, 0);
    grid_sizer_info_team->Add(sizer_map_label, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 4);
    grid_sizer_info_team->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_info_team, 1, wxEXPAND, 0);
    wxStaticLine* static_line = new wxStaticLine(this, wxID_ANY);
    grid_sizer_main->Add(static_line, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 4);
    grid_sizer_button->Add(button_update, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 16);
    grid_sizer_button->Add(button_close, 0, wxALL, 4);
    grid_sizer_main->Add(grid_sizer_button, 1, wxBOTTOM|wxALIGN_RIGHT, 4);
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
    //ListAdjustSize(event.GetSize());
    ListAdjustSize(list_ctrl_extended->GetClientSize());
    m_sizerMapLabel->SetMinSize(m_sizerMap->GetSize().x,-1);

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
    else if (!m_info->m_players && list_ctrl_extended->GetItemCount())
        list_ctrl_extended->DeleteAllItems();
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
            ShowPanelMap(event.IsChecked(),true);
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
        {
            wxString s;
            wxListItem item;
            CslPlayerStats *stats=info->m_playerStats;
            CslPlayerStatsData *data=NULL;

            item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

            loopv(stats->m_stats)
            {
                data=stats->m_stats[i];
                if (!data->m_ok)
                    break;
                if (i<list_ctrl_extended->GetItemCount())
                    continue;

                item.SetId(i);
                list_ctrl_extended->InsertItem(item);
                list_ctrl_extended->SetItemData(i,(long)data);

                if (data->m_player.IsEmpty())
                    s=_("- connecting -");
                else
                    s=data->m_player;
                list_ctrl_extended->SetItem(i,0,s);

                if (data->m_state==CSL_PLAYER_STATE_SB_SPECTATOR)
                    s=_("Spectator");
                else
                    s=data->m_team;
                list_ctrl_extended->SetItem(i,1,s);

                if (data->m_frags<0)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->m_frags);
                list_ctrl_extended->SetItem(i,2,s);

                if (data->m_deaths<0)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->m_deaths);
                list_ctrl_extended->SetItem(i,3,s);

                if (data->m_teamkills<0)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->m_teamkills);
                list_ctrl_extended->SetItem(i,4,s);

                if (data->m_state==CSL_PLAYER_STATE_SB_DEAD)
                    s=_("dead");
                else if (data->m_state==CSL_PLAYER_STATE_SB_EDITING)
                    s=_("editing");
                else if (data->m_state)
                    s=wxT("0");
                else
                    s=s=wxString::Format(wxT("%d"),data->m_health);
                list_ctrl_extended->SetItem(i,5,s);

                if (data->m_state || data->m_armour<0)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->m_armour);
                list_ctrl_extended->SetItem(i,6,s);

                s=GetWeaponStrSB(data->m_weapon);
                list_ctrl_extended->SetItem(i,7,s);

                wxColour colour=*wxBLACK;
                if (data->m_priv==CSL_PLAYER_PRIV_SB_MASTER)
                    i=CSL_LIST_IMG_GREEN;
                else if (data->m_priv==CSL_PLAYER_PRIV_SB_ADMIN)
                    i=CSL_LIST_IMG_ORANGE;
                else if (data->m_state==CSL_PLAYER_STATE_SB_SPECTATOR)
                    i=CSL_LIST_IMG_GREY;
                else
                    i=CSL_LIST_IMG_TRANS;

                list_ctrl_extended->SetItemImage(item,i);
            }

            stats->m_lastResponse=wxDateTime::Now().GetTicks();

            label_records->SetLabel(wxString::Format(_("%d players (%d left)"),
                                    list_ctrl_extended->GetItemCount(),stats->m_ids.length()));
            ListSort(-1);
            break;
        }

        case CSL_PONG_TYPE_TEAMSTATS:
            SetTeamScore();
            break;

        default:
            break;
    }

    delete pp;
}

static const wxColour team_colours[] =
{
    *wxBLUE,*wxRED,wxColour(64,160,64),wxColour(192,128,0),wxColour(0,0,0),wxColour(255,255,255)
};

void CslDlgExtended::UpdateMap()
{
    if (m_mapInfo.m_mapName!=m_info->m_map)
    {
        if (!m_info->m_map.IsEmpty() &&
            m_mapInfo.LoadMapData(m_info->m_map,CslGame::GetGameName(m_info->m_type),m_info->m_protocol))
        {
            if (!m_mapInfo.m_mapNameFull.IsEmpty())
                label_map->SetLabel(m_mapInfo.m_mapNameFull);
            if (!m_mapInfo.m_author.IsEmpty())
            {
                label_author_prefix->SetLabel(_("by"));
                label_author->SetLabel(m_mapInfo.m_author);
            }

            panel_map->SetMap(m_mapInfo.m_bitmap,checkbox_map->IsChecked());
            ShowPanelMap(checkbox_map->IsChecked(),true);
        }
        else
        {
            panel_map->SetOk(false);
            ShowPanelMap(false,false);
        }
    }
    else if (m_mapInfo.m_mapName.IsEmpty())
    {
        panel_map->SetOk(false);
        ShowPanelMap(false,false);
    }
}

void CslDlgExtended::SetTeamScore()
{
    CslTeamStats *stats;
    CslTeamStatsData *data;
    wxSize size;
    wxString s;
    bool hasBases=false;
    wxInt32 h=-1;
    wxInt32 hs=0;
    wxInt32 c=m_teamLabel.GetCount();

    wxASSERT(m_info && m_info->m_teamStats);
    if (!m_info || !m_info->m_teamStats)
        return;

    stats=m_info->m_teamStats;

    for (wxInt32 ic=0;ic<c;ic++)
    {
        m_teamLabel.Item(ic)->SetLabel(wxT(""));
        m_teamLabel.Item(ic)->SetFont(m_labelFont);
    }

    label_map->SetLabel(m_info->m_map.IsEmpty() ? wxString(_("no map")) : m_info->m_map);
    label_author->SetLabel(wxEmptyString);
    label_author_prefix->SetLabel(wxEmptyString);

    UpdateMap();
    m_mapInfo.ResetBasesColour();

    if (stats->m_teamplay)
    {
        loopv(stats->m_stats)
        {
            data=stats->m_stats[i];

            // TODO i==c, dynamically add labels
            if (!data->m_ok || i==c)
                break;

            if (data->m_score>hs)
            {
                hs=data->m_score;
                h=i;
            }

            if (m_mapInfo.m_basesOk && data->IsCapture())
            {
                hasBases=true;
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
            m_teamLabel.Item(i)->SetLabel(s);
            m_teamLabel.Item(i)->SetForegroundColour(team_colours[i]);

            size=m_teamLabel.Item(i)->GetBestSize();
            if (i%2==0)
                size.x+=40;
            else
                size.x+=6;
            m_teamLabel.Item(i)->SetMinSize(size);
        }

        // TODO h<c, dynamically add labels
        if (h>-1 && h<c)
        {
            data=stats->m_stats[h];

            wxFont font=m_labelFont;
            font.SetWeight(wxFONTWEIGHT_BOLD);
            m_teamLabel.Item(h)->SetFont(font);

            if (data->m_score==10000 || stats->m_remain==0)
            {
                s=data->m_team;
                s+=wxT(": ");
                if (data->m_score==10000)
                    s+=_("WINNER");
                else
                    s+=wxString::Format(wxT("%d - "),data->m_score)+_("WINNER");

                m_teamLabel.Item(h)->SetLabel(s);
                m_teamLabel.Item(h)->SetMinSize(m_teamLabel.Item(h)->GetBestSize());

                if (checkbox_update_end->IsChecked())
                    checkbox_update->SetValue(false);
            }
        }
    }
    else
    {
        label_team1->SetLabel(_("Not a team game."));
        label_team1->SetMinSize(label_team1->GetBestSize());
    }

    panel_map->UpdateBases(m_mapInfo.m_bases,hasBases);

    s=FormatSeconds(stats->m_remain>-1 ? stats->m_remain*60 : 0,true,true);
    if (s.IsEmpty())
        s=_("Time is up");
    label_remaining->SetLabel(s);
    label_mode->SetLabel(m_info->m_gameMode);

    m_gridSizerInfo->Layout();
    RecalcMinWidth();
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
            list_ctrl_extended->DeleteAllItems();
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

void CslDlgExtended::RecalcMinWidth(bool forcemin)
{
    wxSize size=GetBestSize();
    size.y=-1;

//#ifndef __WXGTK__
    if (forcemin || GetSize().x<size.x)
        SetSize(size);
//#endif
    SetMinSize(size);
    m_gridSizerMain->SetSizeHints(this);
}

void CslDlgExtended::ListAdjustSize(const wxSize& size)
{
    if (list_ctrl_extended->GetColumnCount()<6)
        return;

    wxInt32 w=size.x-25;

    list_ctrl_extended->SetColumnWidth(0,(wxInt32)(w*0.23f));
    list_ctrl_extended->SetColumnWidth(1,(wxInt32)(w*0.11f));
    list_ctrl_extended->SetColumnWidth(2,(wxInt32)(w*0.11f));
    list_ctrl_extended->SetColumnWidth(3,(wxInt32)(w*0.11f));
    list_ctrl_extended->SetColumnWidth(4,(wxInt32)(w*0.11f));
    list_ctrl_extended->SetColumnWidth(5,(wxInt32)(w*0.11f));
    list_ctrl_extended->SetColumnWidth(6,(wxInt32)(w*0.11f));
    list_ctrl_extended->SetColumnWidth(7,(wxInt32)(w*0.11f));
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
        list_ctrl_extended->GetColumn(column,item);

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
        list_ctrl_extended->SetColumn(m_sortHelper.m_sortType,item);

        item.SetImage(img);
        list_ctrl_extended->SetColumn(column,item);

        m_sortHelper.m_sortType=column;
    }

    if (list_ctrl_extended->GetItemCount()>0)
        list_ctrl_extended->SortItems(ListSortCompareFunc,(long)&m_sortHelper);
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
    list_ctrl_extended->SetColumn(m_sortHelper.m_sortType,item);
}

void CslDlgExtended::ShowPanelMap(const bool show, const bool checkbox)
{
    if (!(show&&m_gridSizerList->GetItem(m_sizerMap)))
    {
        if (show)
        {
            m_gridSizerList->Add(m_sizerMap,1,wxALL|wxEXPAND,3);
            //m_gridSizerInfo->Add(m_sizerMapLabel,1,wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND,4);
            m_gridSizerList->Show(m_sizerMap);
            //m_gridSizerInfo->Show(m_sizerMapLabel);
        }
        else
        {
            m_gridSizerList->Hide(m_sizerMap);
            //m_gridSizerInfo->Hide(m_sizerMapLabel);
            m_gridSizerList->Detach(m_sizerMap);
            //m_gridSizerInfo->Detach(m_sizerMapLabel);

            m_sizerMapLabel->SetMinSize(-1,-1);
        }

        m_gridSizerMain->Layout();
        RecalcMinWidth(true);
        ListAdjustSize(list_ctrl_extended->GetClientSize());
    }

    if (!show)
        checkbox_map->SetValue(show);
    checkbox_map->Enable(checkbox);
}

void CslDlgExtended::ListInit(CslEngine *engine)
{
    wxListItem item;

    m_engine=engine;

    list_ctrl_extended->SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    list_ctrl_extended->InsertColumn(0,item);
    list_ctrl_extended->SetColumn(0,item);

    item.SetText(_("Team"));
    list_ctrl_extended->InsertColumn(1,item);
    list_ctrl_extended->SetColumn(1,item);

    item.SetText(_("Frags"));
    list_ctrl_extended->InsertColumn(2,item);
    list_ctrl_extended->SetColumn(2,item);

    item.SetText(_("Deaths"));
    list_ctrl_extended->InsertColumn(3,item);
    list_ctrl_extended->SetColumn(3,item);

    item.SetText(_("Teamkills"));
    list_ctrl_extended->InsertColumn(4,item);
    list_ctrl_extended->SetColumn(4,item);

    item.SetText(_("Health"));
    list_ctrl_extended->InsertColumn(5,item);
    list_ctrl_extended->SetColumn(5,item);

    item.SetText(_("Armour"));
    list_ctrl_extended->InsertColumn(6,item);
    list_ctrl_extended->SetColumn(6,item);

    item.SetText(_("Weapon"));
    list_ctrl_extended->InsertColumn(7,item);
    list_ctrl_extended->SetColumn(7,item);

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
        list_ctrl_extended->DeleteAllItems();
        label_records->SetLabel(wxString::Format(_("%d players (%d left)"),0,0));
    }

    m_info=info;

    SetTeamScore();
    button_update->SetLabel(_("&Update"));
    button_update->Enable(false);
    label_mode->SetLabel(m_info->m_gameMode);

    s=FormatSeconds(m_info->m_timeRemain>-1 ? m_info->m_timeRemain*60 : 0,true,true);
    if (s.IsEmpty())
        s=_("Time is up");
    label_remaining->SetLabel(s);

    s=_("CSL - Extended info"); s+=wxT(": ");
    s+=m_info->GetBestDescription();
    SetTitle(s);

    QueryInfo();

    if (IsShown())
        return;

    //ListAdjustSize(list_ctrl_extended->GetClientSize());
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
