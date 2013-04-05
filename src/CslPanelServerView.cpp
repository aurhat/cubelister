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
#include "CslApp.h"
#include "CslMenu.h"
#include "CslGeoIP.h"
#include "CslSettings.h"
#include "CslGameConnection.h"
#include "CslPanelServerView.h"

enum
{
    COLUMN_NAME = 0,
    COLUMN_TEAM,
    COLUMN_FRAGS,
    COLUMN_DEATHS,
    COLUMN_TEAMKILLS,
    COLUMN_PING,
    COLUMN_KPD,
    COLUMN_ACCURACY,
    COLUMN_HEALTH,
    COLUMN_ARMOUR,
    COLUMN_WEAPON
};

BEGIN_EVENT_TABLE(CslPanelServerView,wxPanel)
    EVT_SIZE(CslPanelServerView::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslListCtrlPlayerView, CslListCtrl)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, CslListCtrlPlayerView::OnItemSelected)
    CSL_EVT_LIST_COLUMN_TOGGLED(wxID_ANY,CslListCtrlPlayerView::OnColumnToggle)
    CSL_EVT_LIST_ALL_ITEMS_SELECTED(wxID_ANY, CslListCtrlPlayerView::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, CslListCtrlPlayerView::OnItemDeselected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, CslListCtrlPlayerView::OnItemActivated)
    EVT_CONTEXT_MENU(CslListCtrlPlayerView::OnContextMenu)
    EVT_MENU(wxID_ANY, CslListCtrlPlayerView::OnMenu)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CslPanelServerView, wxPanel)

CslPanelServerView::CslPanelServerView(wxWindow* parent, const wxString& listname, long liststyle)
        : wxPanel(parent,wxID_ANY)
{
    m_sizer = new wxFlexGridSizer(2, 1, 0, 0);
    m_list_ctrl = new CslListCtrlPlayerView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                            liststyle, wxDefaultValidator, listname);
    m_label = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_sizer->Add(m_list_ctrl, 0, wxALL|wxEXPAND, 0);
    m_sizer->Add(m_label, 0, wxALL|wxEXPAND, 2);
    m_sizer->AddGrowableRow(0);
    m_sizer->AddGrowableCol(0);
    SetSizer(m_sizer);
    m_sizer->Fit(this);
    m_sizer->Layout();
}

void CslPanelServerView::OnSize(wxSizeEvent& event)
{
    wxSize size=event.GetSize();

    m_label->SetLabel(GetLabelText());
    m_label->Wrap(size.x-4);
#ifdef __WXMAC__
    size.y-=m_label->GetBestSize().y+4;
    m_list_ctrl->SetSize(size);
#endif //__WXMAC__
    //m_list_ctrl->ListAdjustSize(event.GetSize());
#ifdef __WXMAC__
    //fixes flicker after resizing
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif //__WXMAC__

    event.Skip();
}

wxString CslPanelServerView::GetLabelText()
{
    CslServerInfo *info = m_list_ctrl->GetServerInfo();

    if (!info)
        return wxEmptyString;

    wxString s;

    if (!info->GameMode.IsEmpty())
        s << info->GameMode+_(" on ");
    if (!info->Map.IsEmpty())
        s << info->Map+wxT(" ");
    if (info->TimeRemain>0)
        s << wxT("(") << FormatSeconds(info->TimeRemain, false, false) << wxT(")");

    s.Replace(wxT("&"), wxT("&&"));

    return s;
}

void CslPanelServerView::UpdateData()
{
    // static text flicker fix at least on __WXMSW__
    wxWindowUpdateLocker lock(this);

    m_label->SetLabel(GetLabelText());
    m_label->Wrap(GetSize().x-4);
#ifdef __WXMAC__
    wxSize size=m_list_ctrl->GetSize();
    size.y-=m_label->GetBestSize().y+4;
    m_list_ctrl->SetSize(size);
#endif
    // first update the list control, then resize
    m_list_ctrl->UpdateData();

    m_sizer->Layout();
}

void CslPanelServerView::CheckServerStatus()
{
    CslServerInfo *info;

    if (!(info = m_list_ctrl->GetServerInfo()))
        return;

    bool enable = CslEngine::PingOk(*info, CslGetSettings().UpdateInterval);

    if (!enable && m_label->IsEnabled())
    {
        m_list_ctrl->EnableEntries(false);
        m_label->Disable();
    }
    else if (enable && !m_label->IsEnabled())
    {
        m_label->Enable();
        m_list_ctrl->EnableEntries(true);
    }
}


wxSize CslListCtrlPlayerView::BestSizeMicro(140,350);
wxSize CslListCtrlPlayerView::BestSizeMini(280,350);

IMPLEMENT_DYNAMIC_CLASS(CslListCtrlPlayerView, CslListCtrl)

CslListCtrlPlayerView::CslListCtrlPlayerView(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator,const wxString& name) :
        CslListCtrl(parent, id, pos, size, style, validator, name),
        m_view(-1), m_info(NULL)
{
}

CslListCtrlPlayerView::~CslListCtrlPlayerView()
{
    loopv(m_selected)
        delete (CslPlayerStatsData*)m_selected[i];

    m_selected.Empty();
}

void CslListCtrlPlayerView::OnColumnToggle(wxListEvent& WXUNUSED(event))
{
    UpdateData();
}

void CslListCtrlPlayerView::OnItemSelected(wxListEvent& event)
{
    CslPlayerStatsData *d=(CslPlayerStatsData*)GetItemData(event.GetIndex());
    m_selected.Add(new CslPlayerStatsData(*d));
}

void CslListCtrlPlayerView::OnItemDeselected(wxListEvent& event)
{
    CslPlayerStatsData *id = (CslPlayerStatsData*)GetItemData(event.GetIndex());

    loopi(m_selected.GetCount())
    {
        CslPlayerStatsData *d = (CslPlayerStatsData*)m_selected.Item(i);

        if (*d==*id)
        {
            m_selected.RemoveAt(i);
            delete d;
            break;
        }
    }
}

void CslListCtrlPlayerView::OnItemActivated(wxListEvent& event)
{
    if (!m_info)
        return;

    event.SetClientData((void*)m_info);
    event.Skip();
}

void CslListCtrlPlayerView::OnContextMenu(wxContextMenuEvent& event)
{
    if (!m_info)
        return;

    wxMenu menu;
    wxPoint point=event.GetPosition();

    CSL_MENU_CREATE_CONNECT(menu,m_info)
    CSL_MENU_CREATE_EXTINFO(menu,m_info,m_view);
    menu.AppendSeparator();
    CSL_MENU_CREATE_SRVMSG(menu,m_info);
    menu.AppendSeparator();
    CSL_MENU_CREATE_URICOPY(menu)
    menu.AppendSeparator();
    CSL_MENU_CREATE_NOTIFY(menu,m_info)
    menu.AppendSeparator();
    if (m_selected.GetCount()==1)
    {
        CSL_MENU_CREATE_LOCATION(menu)
        menu.AppendSeparator();
    }
    CSL_MENU_CREATE_SAVEIMAGE(menu)

    // notify plugins
    CslPluginEvent evt(CslPluginEvent::EVT_PLAYER_MENU);
    evt.SetMenu(&menu);
    evt.SetServerInfo(m_info);
    evt.SetPlayerStatsData(m_selected.GetCount()==1 ? (CslPlayerStatsData*)m_selected[0] : NULL);
    ::wxGetApp().GetTopWindow()->GetEventHandler()->ProcessEvent(evt);

    //from keyboard
    if (point==wxDefaultPosition)
        point=wxGetMousePosition();
    point=ScreenToClient(point);

    PopupMenu(&menu,point);
}

void CslListCtrlPlayerView::OnMenu(wxCommandEvent& event)
{
    wxInt32 id=event.GetId();

    CSL_MENU_EVENT_SKIP_CONNECT(id,m_info)
    CSL_MENU_EVENT_SKIP_EXTINFO(id,m_info)
    CSL_MENU_EVENT_SKIP_SRVMSG(id,m_info)
    CSL_MENU_EVENT_SKIP_NOTIFY(id,m_info)

    if (CSL_MENU_EVENT_IS_URICOPY(id))
    {
        wxArrayPtrVoid *servers=new wxArrayPtrVoid;
        servers->Add((void*)m_info);
        event.SetClientData((void*)servers);
    }
    else if (CSL_MENU_EVENT_IS_LOCATION(id))
        event.SetClientData((void*)new wxString(NtoA(((CslPlayerStatsData*)m_selected.Item(0))->IP)));

        event.Skip();
    }

inline wxString& CslListCtrlPlayerView::FormatStats(wxString& in, CslPlayerStatsData *data, int type)
{
    in.Empty();

    switch (type)
    {
        case CslGame::PLAYER_STATS_NAME:
            if (data->Name.IsEmpty())
                in<<_("- connecting -");
            else
                in<<data->Name;
            break;

        case CslGame::PLAYER_STATS_TEAM:
            if (data->State==CSL_PLAYER_STATE_SPECTATOR)
                in<<_("Spectator");
            else
                in<<data->Team;
            break;

        case CslGame::PLAYER_STATS_FRAGS:
            if (data->Flagscore!=0)
                in<<wxString::Format(wxT("%d / %d"), data->Frags, data->Flagscore);
            else
                in<<data->Frags;
            break;

        case CslGame::PLAYER_STATS_DEATHS:
            in<<data->Deaths;
            break;

        case CslGame::PLAYER_STATS_TEAMKILLS:
            in<<data->Teamkills;
            break;

        case CslGame::PLAYER_STATS_PING:
            if (data->Ping<0)
                in<<wxT("no data");
            else if (data->Ping>=9999)
                in<<wxT("LAG");
            else
                in<<data->Ping;
            break;

        case CslGame::PLAYER_STATS_KPD:
            in<<wxString::Format(wxT("%.2f"), data->KpD);
            break;

        case CslGame::PLAYER_STATS_ACCURACY:
            in<<wxString::Format(wxT("%d%%"), data->Accuracy);
            break;

        case CslGame::PLAYER_STATS_HEALTH:
            if (data->State==CSL_PLAYER_STATE_UNKNOWN)
                in<<_("no data");
            else if (data->State==CSL_PLAYER_STATE_DEAD)
                in<<_("dead");
            else if (data->State==CSL_PLAYER_STATE_EDITING)
                in<<_("editing");
            else if (data->State)
                in<<0;
            else
                in<<data->Health;
            break;

        case CslGame::PLAYER_STATS_ARMOUR:
            if (data->State==CSL_PLAYER_STATE_UNKNOWN)
                in<<_("no data");
            else if (data->State || data->Armour<0)
                in<<0;
            else
                in<<data->Armour;
            break;

        case CslGame::PLAYER_STATS_WEAPON:
            in<<m_info->GetGame().GetWeaponName(data->Weapon, m_info->Protocol);
            break;

        default: break;
    }

    return in;
}

void CslListCtrlPlayerView::GetToolTipText(wxInt32 row,CslToolTipEvent& event)
{
    if (row>=0 && row<GetItemCount())
    {
        CslPlayerStatsData *data=(CslPlayerStatsData*)GetItemData(row);

        if (!data)
            return;

        wxString s;
        const wxString *descriptions;
        wxInt32 c = m_info->GetGame().GetPlayerstatsDescriptions(&descriptions);

        event.Title=_("Player information");

        loopi(c)
        {
            if (!FormatStats(s, data, i).IsEmpty())
            {
                event.Text.Add(descriptions[i]);
                event.Text.Add(s);
            }
        }

        event.Text.Add(_("Country"));

        wxString city=CslGeoIP::GetCityNameByIPnum(data->IP);
        wxString location=CslGeoIP::GetCountryNameByIPnum(data->IP);

        if (!CslGeoIP::IsOk())
            location=_("No GeoIP database loaded.");
        else if (IsLocalIPV4(data->IP))
            location=_("local network");
        else
        {
            if (!location.IsEmpty())
            {
                if (!city.IsEmpty())
                    location<<wxT(" (")<<city<<wxT(")");
            }
            else
                location=_("Unknown location.");
        }

        event.Text.Add(location);

        event.Text.Add(wxT("ID / IP"));
        event.Text.Add(wxString::Format(wxT("%d / %d.%d.%d.x"),data->ID,
                                        data->IP&0xff, data->IP>>8&0xff, data->IP>>16&0xff));
    }

}

wxString CslListCtrlPlayerView::GetScreenShotFileName()
{
    wxString s;

    s<<m_info->GetBestDescription()<<wxT("-")<<m_info->GameMode<<wxT("-")<<m_info->Map;
    FixFilename(s);
    s<<wxT("-")<<wxDateTime::Now().Format(wxT("%Y%m%d_%H%M%S"))<<wxT(".png");

    return s;
}

void CslListCtrlPlayerView::UpdateData()
{
    if (!m_info)
    {
        ListClear();
        return;
    }

    wxString s;
    wxInt32 i, j;
    wxListItem item;
    CslPlayerStatsData *data;
    const wxString *descriptions;
    CslEngine *engine = ::wxGetApp().GetCslEngine();
    const CslPlayerStats& stats = m_info->PlayerStats;
    wxInt32 dlen = m_info->GetGame().GetPlayerstatsDescriptions(&descriptions);

    // check if parent already did it
    bool freeze = !IsFrozen();

    if (freeze)
        Freeze();

    for (i = 0; i<GetItemCount(); i++)
        ListDeselectItem(i);

    for (i=0; i<(wxInt32)stats.m_stats.size(); i++)
    {
        data=stats.m_stats[i];

        if (!data->Ok)
        {
            i--;
            break;
        }

        for (j=0; j<dlen; j++)
        {
            wxInt32 column, image = -1;

            switch (j)
            {
                case CslGame::PLAYER_STATS_NAME:      column = COLUMN_NAME; image = GetCountryFlag(engine, data->IP); break;
                case CslGame::PLAYER_STATS_TEAM:      column = COLUMN_TEAM;                                           break;
                case CslGame::PLAYER_STATS_FRAGS:     column = COLUMN_FRAGS;                                          break;
                case CslGame::PLAYER_STATS_DEATHS:    column = COLUMN_DEATHS;                                         break;
                case CslGame::PLAYER_STATS_TEAMKILLS: column = COLUMN_TEAMKILLS;                                      break;
                case CslGame::PLAYER_STATS_PING:      column = COLUMN_PING;                                           break;
                case CslGame::PLAYER_STATS_KPD:       column = COLUMN_KPD;                                            break;
                case CslGame::PLAYER_STATS_ACCURACY:  column = COLUMN_ACCURACY;                                       break;
                case CslGame::PLAYER_STATS_HEALTH:    column = COLUMN_HEALTH;                                         break;
                case CslGame::PLAYER_STATS_ARMOUR:    column = COLUMN_ARMOUR;                                         break;
                case CslGame::PLAYER_STATS_WEAPON:    column = COLUMN_WEAPON;                                         break;
                default:                              column = -1;                                                    break;
            }

            if (column>=0)
            {
                ListSetItem(i, column, FormatStats(s, data, j), image);
                SetItemPtrData(i, (wxUIntPtr)data);
            }
        }

        item.SetId(i);

        wxInt32 priv = m_info->GetGame().GetPrivileges(data->Privileges, m_info->Protocol);

        if (priv==CSL_PLAYER_PRIV_MASTER)
            SetItemBackgroundColour(item, CslGetSettings().ColPlayerMaster);
        else if (priv==CSL_PLAYER_PRIV_AUTH)
            SetItemBackgroundColour(item, CslGetSettings().ColPlayerAuth);
        else if (priv==CSL_PLAYER_PRIV_ADMIN)
            SetItemBackgroundColour(item, CslGetSettings().ColPlayerAdmin);
        else if (data->State==CSL_PLAYER_STATE_SPECTATOR)
            SetItemBackgroundColour(item, CslGetSettings().ColPlayerSpectator);
        else
            SetItemBackgroundColour(item,GetBackgroundColour());
    }

    for (j = GetItemCount()-1; j>i; j--)
    {
        item.SetId(j);
        DeleteItem(item);
    }

    for (i = m_selected.GetCount()-1; i>=0; i--)
    {
        data=(CslPlayerStatsData*)m_selected.Item(i);

        if ((j = ListFindItem(data))==wxNOT_FOUND)
        {
            delete data;
            m_selected.RemoveAt(i);
        }
        else
            ListSelectItem(j);
    }

    ListSort();

    if (freeze)
        Thaw();
}

void CslListCtrlPlayerView::EnableEntries(bool enable)
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    if (!m_info)
    {
        ListClear();
        return;
    }

    wxListItem item;

    for (wxInt32 i=0, j=GetItemCount(); i<j; i++)
    {
        item.SetId(i);
        SetItemTextColour(item, CSL_SYSCOLOUR(enable ? wxSYS_COLOUR_WINDOWTEXT : wxSYS_COLOUR_GRAYTEXT));
    }
}

void CslListCtrlPlayerView::ListClear()
{
    DeleteAllItems();

    for (wxInt32 i=0, j=m_selected.GetCount(); i>j; i++)
        delete(CslPlayerStatsData*)m_selected[i];

    m_selected.Empty();
}

void CslListCtrlPlayerView::SetServerInfo(CslServerInfo *info)
{
    m_info=info;

    ListClear();

    if (!info)
        return;

    const wxString *desc;
    wxInt32 c = info->GetGame().GetPlayerstatsDescriptions(&desc);

    loopi(c)
        ListSetColumn(i, desc[i]);
}

void CslListCtrlPlayerView::ListInit(wxInt32 view)
{
    bool enabled=true;
    wxUint32 columns=CslSettings::GetListSettings(GetName()).ColumnMask;

    m_view=view;

    #define COLENABLED(id) (columns>0 ? CSL_FLAG_CHECK(columns, 1<<(id)) : enabled)
    ListAddColumn(_("Player"),    wxLIST_FORMAT_LEFT, 3.0f, true, true);
    enabled=m_view>=SIZE_MINI;
    ListAddColumn(_("Team"),      wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_TEAM));
    ListAddColumn(_("Frags"),     wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_FRAGS));
    ListAddColumn(_("Deaths"),    wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_DEATHS));
    ListAddColumn(_("Teamkills"), wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_TEAMKILLS));
    enabled=m_view>=SIZE_DEFAULT;
    ListAddColumn(_("Ping"),      wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_PING));
    ListAddColumn(_("KpD"),       wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_KPD));
    ListAddColumn(_("Accuracy"),  wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_ACCURACY));
    ListAddColumn(_("Health"),    wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_HEALTH));
    ListAddColumn(_("Armour"),    wxLIST_FORMAT_LEFT, 1.0f, COLENABLED(COLUMN_ARMOUR));
    ListAddColumn(_("Weapon"),    wxLIST_FORMAT_LEFT, 2.0f, COLENABLED(COLUMN_WEAPON));
    #undef COLENABLED

    wxInt32 mode, column;

    if (!ListColumnIsEnabled(COLUMN_FRAGS))
    {
        mode=CslListSort::SORT_ASC;
        column=COLUMN_NAME;
    }
    else
    {
        mode=CslListSort::SORT_DSC;
        column=COLUMN_FRAGS;
    }

    SetSortCallback(ListSortCompareFunc, mode, column);
}

int wxCALLBACK CslListCtrlPlayerView::ListSortCompareFunc(long item1, long item2, long data)
{
    CslPlayerStatsData *data1=(CslPlayerStatsData*)item1;
    CslPlayerStatsData *data2=(CslPlayerStatsData*)item2;
    CslListSort *helper=(CslListSort*)data;

    if (!data || !data1 || !data2)
        return 0;

    wxInt32 ret=0;

    if (helper->Column!=COLUMN_NAME)
    {
        if (data1->State==CSL_PLAYER_STATE_SPECTATOR)
            return 1;
        if (data2->State==CSL_PLAYER_STATE_SPECTATOR)
            return -1;
    }

#define SORT_BY_FRAGS(data1, data2, _mode) \
    if (!(ret=helper->Cmp(data1->Frags, data2->Frags, _mode))) \
    { \
        if (!(ret=helper->Cmp(data1->Accuracy, data2->Accuracy, \
                              CslListSort::SORT_DSC))) \
        { \
            if (!(ret=helper->Cmp(data1->Deaths, data2->Deaths, \
                                  CslListSort::SORT_ASC))) \
            { \
                ret=helper->Cmp(data1->Teamkills, data2->Teamkills, \
                                CslListSort::SORT_ASC); \
            } \
        } \
    }

    switch (helper->Column)
    {
        case COLUMN_NAME:
            ret=helper->Cmp(data1->Name,data2->Name);
            break;

        case COLUMN_TEAM:
            ret=helper->Cmp(data1->Team,data2->Team);
            break;

        case COLUMN_FRAGS:
            SORT_BY_FRAGS(data1, data2, helper->Mode)
            break;

        case COLUMN_DEATHS:
            ret=helper->Cmp(data1->Deaths,data2->Deaths);
            break;

        case COLUMN_TEAMKILLS:
            ret=helper->Cmp(data1->Teamkills,data2->Teamkills);
            break;

        case COLUMN_PING:
            ret=helper->Cmp(data1->Ping,data2->Ping);
            break;

        case COLUMN_KPD:
            ret=helper->Cmp(data1->KpD, data2->KpD);
            break;

        case COLUMN_ACCURACY:
            ret=helper->Cmp(data1->Accuracy,data2->Accuracy);
            break;

        case COLUMN_HEALTH:
            ret=helper->Cmp(data1->Health,data2->Health);
            break;

        case COLUMN_ARMOUR:
            ret=helper->Cmp(data1->Armour,data2->Armour);
            break;

        case COLUMN_WEAPON:
            ret=helper->Cmp(data1->Weapon,data2->Weapon);
            break;

        default:
            break;
    }

    if (!ret && helper->Column!=COLUMN_FRAGS)
        SORT_BY_FRAGS(data1, data2, CslListSort::SORT_DSC)

#undef SORT_BY_FRAGS
    return ret;
}
