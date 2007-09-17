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
#ifndef _MSC_VER
#include "img/sortasc_16.xpm"
#include "img/sortdsc_16.xpm"
#endif

BEGIN_EVENT_TABLE(CslDlgExtended, wxDialog)
    EVT_CLOSE(CslDlgExtended::OnClose)
    EVT_SIZE(CslDlgExtended::OnSize)
    EVT_LIST_COL_CLICK(wxID_ANY,CslDlgExtended::OnColumnLeftClick)
    EVT_TIMER(wxID_ANY,CslDlgExtended::OnTimer)
    EVT_BUTTON(wxID_ANY,CslDlgExtended::OnCommandEvent)
    CSL_EVT_PING_STATS(wxID_ANY,CslDlgExtended::OnPingStats)
END_EVENT_TABLE()


enum { CSL_LIST_IMG_SORT_ASC = 0, CSL_LIST_IMG_SORT_DSC };
enum { SORT_RANK = 0,SORT_PLAYER, SORT_TEAM, SORT_FRAGS, SORT_DEATHS, SORT_TEAMKILLS };

enum { BUTTON_REFRESH = wxID_HIGHEST +1 };


CslDlgExtended::CslDlgExtended(wxWindow* parent,int id,const wxString& title,
                               const wxPoint& pos, const wxSize& size, long style):
        wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
        m_engine(NULL),m_info(NULL)
{
    // begin wxGlade: CslDlgExtended::CslDlgExtended
    list_ctrl_extended = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
    label_status = new wxStaticText(this, wxID_ANY, _("Waiting for data..."));
    button_refresh = new wxButton(this, BUTTON_REFRESH, _("&Refresh"));
    button_close = new wxButton(this, wxID_CLOSE, _("&Close"));

    set_properties();
    do_layout();
    // end wxGlade

    m_imageList.Create(16,16,true);
#ifndef _MSC_VER
    m_imageList.Add(wxBitmap(sortasc_16_xpm));
    m_imageList.Add(wxBitmap(sortdsc_16_xpm));
#else
    m_imageList.Add(wxIcon(wxT("ICON_LIST_ASC"),wxBITMAP_TYPE_ICO_RESOURCE));
    m_imageList.Add(wxIcon(wxT("ICON_LIST_DSC"),wxBITMAP_TYPE_ICO_RESOURCE));
#endif

    m_timer.SetOwner(this);
}

void CslDlgExtended::set_properties()
{
    // begin wxGlade: CslDlgExtended::set_properties
    SetTitle(_("CSL - Extended info"));
    list_ctrl_extended->SetMinSize(wxSize(450,300));
    // end wxGlade
}

void CslDlgExtended::do_layout()
{
    // begin wxGlade: CslDlgExtended::do_layout
    wxFlexGridSizer* grid_sizer_main = new wxFlexGridSizer(2, 1, 0, 0);
    wxFlexGridSizer* grid_sizer_button = new wxFlexGridSizer(1, 5, 0, 0);
    grid_sizer_main->Add(list_ctrl_extended, 1, wxALL|wxEXPAND, 4);
    wxStaticText* label_status_static = new wxStaticText(this, wxID_ANY, _("Status:"));
    grid_sizer_button->Add(label_status_static, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 8);
    grid_sizer_button->Add(label_status, 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);
    grid_sizer_button->Add(button_refresh, 0, wxALL, 4);
    grid_sizer_button->Add(8, 1, 0, wxADJUST_MINSIZE, 0);
    grid_sizer_button->Add(button_close, 0, wxALL, 4);
    grid_sizer_button->AddGrowableCol(1);
    grid_sizer_main->Add(grid_sizer_button, 1, wxEXPAND, 0);
    SetSizer(grid_sizer_main);
    grid_sizer_main->Fit(this);
    grid_sizer_main->AddGrowableRow(0);
    grid_sizer_main->AddGrowableCol(0);
    Layout();
    // end wxGlade

    grid_sizer_main->SetSizeHints(this);
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
    ListAdjustSize(event.GetSize());
    event.Skip();
}

void CslDlgExtended::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslDlgExtended::OnTimer(wxTimerEvent& event)
{
    button_refresh->Enable();
}

void CslDlgExtended::OnCommandEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxID_CLOSE:
            Close();
            break;

        case BUTTON_REFRESH:
            QueryInfo();
            break;
    }

    event.Skip();
}

void CslDlgExtended::OnPingStats(wxCommandEvent& event)
{
    CslServerInfo *info=(CslServerInfo*)event.GetClientData();

    if (!m_info || !info || m_info!=info)
        return;

    list_ctrl_extended->DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
    wxString s;
    CslPlayerStats *stats;
    wxUint32 c=0;

    loopv(info->m_playerStats)
    {
        stats=info->m_playerStats.at(i);
        if (!stats->m_ok)
            break;

        item.SetId(i);
        list_ctrl_extended->InsertItem(item);
        list_ctrl_extended->SetItem(i,0,wxT(""));
        list_ctrl_extended->SetItem(i,1,stats->m_player);
        list_ctrl_extended->SetItem(i,2,stats->m_team);
        if (stats->m_frags<0)
            s=_("no data");
        else
            s=wxString::Format(wxT("%d"),stats->m_frags);
        list_ctrl_extended->SetItem(i,3,s);

        if (stats->m_deaths<0)
            s=_("no data");
        else
            s=wxString::Format(wxT("%d"),stats->m_deaths);
        list_ctrl_extended->SetItem(i,4,s);

        if (stats->m_teamkills<0)
            s=_("no data");
        else
            s=wxString::Format(wxT("%d"),stats->m_teamkills);
        list_ctrl_extended->SetItem(i,5,s);

        list_ctrl_extended->SetItemData(i,(long)stats);
        c++;
    }

    label_status->SetLabel(wxString::Format(wxT("Got %d records"),c));
    ListSort(-1);
}

void CslDlgExtended::QueryInfo()
{
    if (!m_timer.IsRunning())
    {
        label_status->SetLabel(_("Waiting for data..."));
        button_refresh->Enable(false);
        m_info->InvalidateStats();
        m_engine->PingStats(m_info);
        m_timer.Start(15000,true);
    }
}

void CslDlgExtended::ListAdjustSize(wxSize size)
{
    wxInt32 w=size.x-25;

    list_ctrl_extended->SetColumnWidth(0,(wxInt32)(w*0.15f));
    list_ctrl_extended->SetColumnWidth(1,(wxInt32)(w*0.25f));
    list_ctrl_extended->SetColumnWidth(2,(wxInt32)(w*0.15f));
    list_ctrl_extended->SetColumnWidth(3,(wxInt32)(w*0.15f));
    list_ctrl_extended->SetColumnWidth(4,(wxInt32)(w*0.15f));
    list_ctrl_extended->SetColumnWidth(5,(wxInt32)(w*0.15f));
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

void CslDlgExtended::ListInit(CslEngine *engine)
{
    wxListItem item;

    m_engine=engine;

    list_ctrl_extended->SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);

    item.SetMask(wxLIST_MASK_TEXT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Rank"));
    list_ctrl_extended->InsertColumn(0,item);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    list_ctrl_extended->InsertColumn(1,item);

    item.SetText(_("Team"));
    list_ctrl_extended->InsertColumn(2,item);

    item.SetAlign(wxLIST_FORMAT_RIGHT);
    item.SetText(_("Frags"));
    list_ctrl_extended->InsertColumn(3,item);

    item.SetText(_("Deaths"));
    list_ctrl_extended->InsertColumn(4,item);

    item.SetText(_("Teamkills"));
    list_ctrl_extended->InsertColumn(5,item);

    ListAdjustSize(GetClientSize());

    m_sortHelper.Init(CSL_SORT_ASC,SORT_RANK);
    ToggleSortArrow();
}

void CslDlgExtended::DoShow(CslServerInfo *info)
{
    if (!info || !m_engine)
    {
        wxASSERT(info && m_engine);
        return;
    }

    if (m_info!=info && m_timer.IsRunning())
        m_timer.Stop();

    m_info=info;

    QueryInfo();

    if (!IsShown())
        Show();
}

int wxCALLBACK CslDlgExtended::ListSortCompareFunc(long item1,long item2,long data)
{
    CslPlayerStats *stats1=(CslPlayerStats*)item1;
    CslPlayerStats *stats2=(CslPlayerStats*)item2;

    wxInt32 type;
    wxInt32 sortMode=((CslListSortHelper*)data)->m_sortMode;
    wxInt32 sortType=((CslListSortHelper*)data)->m_sortType;
    wxInt32 vi1=0,vi2=0;
    wxUint32 vui1=0,vui2=0;
    wxString vs1=wxEmptyString,vs2=wxEmptyString;

    switch (sortType)
    {
        case SORT_RANK:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->m_rank;
            vi2=stats2->m_rank;
            break;

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
