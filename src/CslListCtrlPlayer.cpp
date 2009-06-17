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
#include "CslMenu.h"
#include "CslGeoIP.h"
#include "CslSettings.h"
#include "CslGameConnection.h"
#include "CslListCtrlPlayer.h"

#define CSL_COLOUR_MASTER    wxColour(64,255,128)
#define CSL_COLOUR_ADMIN     wxColour(255,128,0)
#define CSL_COLOUR_SPECTATOR wxColour(192,192,192)

enum
{
    SORT_NAME = 0,
    SORT_TEAM,
    SORT_FRAGS,
    SORT_DEATHS,
    SORT_TEAMKILLS,
    SORT_PING,
    SORT_ACCURACY,
    SORT_HEALTH,
    SORT_ARMOUR,
    SORT_WEAPON
};

BEGIN_EVENT_TABLE(CslPanelPlayer,wxPanel)
    EVT_SIZE(CslPanelPlayer::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslListCtrlPlayer,CslListCtrl)
    EVT_LIST_COL_CLICK(wxID_ANY,CslListCtrlPlayer::OnColumnLeftClick)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,CslListCtrlPlayer::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,CslListCtrlPlayer::OnItemDeselected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlPlayer::OnItemActivated)
    EVT_CONTEXT_MENU(CslListCtrlPlayer::OnContextMenu)
    EVT_MENU(wxID_ANY,CslListCtrlPlayer::OnMenu)
END_EVENT_TABLE()


CslPanelPlayer::CslPanelPlayer(wxWindow* parent,long listStyle)
        : wxPanel(parent,wxID_ANY)
{
    m_sizer=new wxFlexGridSizer(2,1,0,0);

    m_listCtrl=new CslListCtrlPlayer(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,listStyle);
    m_label=new wxStaticText(this,wxID_ANY,wxEmptyString);

    m_sizer->Add(m_listCtrl,0,wxALL|wxEXPAND,0);
    m_sizer->Add(m_label,0,wxALL|wxEXPAND,2);

    m_sizer->AddGrowableRow(0);
    m_sizer->AddGrowableCol(0);

    SetSizer(m_sizer);
    m_sizer->Fit(this);
    m_sizer->Layout();
}

void CslPanelPlayer::OnSize(wxSizeEvent& event)
{
    wxSize size=event.GetSize();

    m_label->SetLabel(GetLabelText());
    m_label->Wrap(size.x-4);
#ifdef __WXMAC__
    size.y-=m_label->GetBestSize().y+4;
    m_listCtrl->SetSize(size);
#endif //__WXMAC__
    m_listCtrl->ListAdjustSize(size);
#ifdef __WXMAC__
    //fixes flicker after resizing
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif //__WXMAC__

    event.Skip();
}

wxString CslPanelPlayer::GetLabelText()
{
    CslServerInfo *info=m_listCtrl->ServerInfo();

    if (!info)
        return wxEmptyString;

    wxString s;

    if (!info->GameMode.IsEmpty())
        s+=info->GameMode+_(" on ");
    if (!info->Map.IsEmpty())
        s+=info->Map+wxT(" ");
    if (info->TimeRemain>0)
        s+=wxString::Format(wxT("(< %d %s)"),info->TimeRemain,info->TimeRemain==1 ?
                            _("Minute"):_("Minutes"))+wxT(" ");
    s.Replace(wxT("&"),wxT("&&"));

    return s;
}

void CslPanelPlayer::UpdateData()
{
#ifdef __WXMSW__
    //fixes flicker of label text
    wxWindowUpdateLocker lock(this);
#endif
    m_label->SetLabel(GetLabelText());
    m_label->Wrap(GetSize().x-4);
#ifdef __WXMAC__
    wxSize size=m_listCtrl->GetSize();
    size.y-=m_label->GetBestSize().y+4;
    m_listCtrl->SetSize(size);
#endif
    m_listCtrl->UpdateData();
    m_sizer->Layout();
}

void CslPanelPlayer::CheckServerStatus()
{
    CslServerInfo *info;

    if (!(info=m_listCtrl->ServerInfo()))
        return;

    bool enable=CslEngine::PingOk(*info,g_cslSettings->updateInterval);

    if (!enable && m_label->IsEnabled())
    {
        m_listCtrl->EnableEntries(false);
        m_label->Disable();
    }
    else if (enable && !m_label->IsEnabled())
    {
        m_label->Enable();
        m_listCtrl->EnableEntries(true);
    }
}


wxSize CslListCtrlPlayer::BestSizeMicro(140,350);
wxSize CslListCtrlPlayer::BestSizeMini(280,350);

CslListCtrlPlayer::CslListCtrlPlayer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator,const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name),
        m_view(-1),m_info(NULL),m_processSelectEvent(true)
{
}

CslListCtrlPlayer::~CslListCtrlPlayer()
{
    WX_CLEAR_ARRAY(m_selected);
}

void CslListCtrlPlayer::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslListCtrlPlayer::OnItemSelected(wxListEvent& event)
{
    if (!m_processSelectEvent)
        return;

    wxListItem item;

    item.SetId(event.GetIndex());
    GetItem(item);

    CslPlayerStatsData *data=(CslPlayerStatsData*)GetItemData(item);
    m_selected.Add(new CslPlayerStatsData(*data));
}

void CslListCtrlPlayer::OnItemDeselected(wxListEvent& event)
{
    if (!m_processSelectEvent)
        return;

    wxUint32 i;
    wxListItem item;

    item.SetId(event.GetIndex());
    GetItem(item);

    CslPlayerStatsData *data,*idata=(CslPlayerStatsData*)GetItemData(item);

    for (i=0;i<m_selected.GetCount();i++)
    {
        data=m_selected.Item(i);

        if (*data==*idata)
        {
            m_selected.RemoveAt(i);
            delete data;
            break;
        }
    }
}

void CslListCtrlPlayer::OnItemActivated(wxListEvent& event)
{
    if (!m_info)
        return;

    event.SetClientData((void*)m_info);
    event.Skip();
}

void CslListCtrlPlayer::OnContextMenu(wxContextMenuEvent& event)
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

    //from keyboard
    if (point.x==-1 && point.y==-1)
        point=wxGetMousePosition();
    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlPlayer::OnMenu(wxCommandEvent& event)
{
    wxInt32 id=event.GetId();

    CSL_MENU_EVENT_SKIP_CONNECT(id,m_info)
    CSL_MENU_EVENT_SKIP_EXTINFO(id,m_info)
    CSL_MENU_EVENT_SKIP_SRVMSG(id,m_info)
    CSL_MENU_EVENT_SKIP_NOTIFY(id,m_info)
    CSL_MENU_EVENT_SKIP_SAVEIMAGE(id)

    if (CSL_MENU_EVENT_IS_LOCATION(id))
    {
        event.SetClientData((void*)new wxString(Int2IP(m_selected.Item(0)->IP)));
        event.Skip();
    }
    else if (CSL_MENU_EVENT_IS_URICOPY(id))
    {
        VoidPointerArray *servers=new VoidPointerArray;
        servers->Add((void*)m_info);
        event.SetClientData((void*)servers);
        event.Skip();
    }
}

void CslListCtrlPlayer::GetToolTipText(wxInt32 row,CslToolTipEvent& event)
{
    if (row<GetItemCount())
    {
        wxInt32 i;
        const char *c;
        wxListItem item,column;

        column.SetMask(wxLIST_MASK_TEXT);
        item.SetMask(wxLIST_MASK_TEXT);
        item.SetId(row);

        for (i=0;i<GetColumnCount();i++)
        {
            item.SetColumn(i);
            GetItem(item);
            GetColumn(i,column);

            const wxString& s=item.GetText();

            if (!s.IsEmpty())
            {
                event.Text.Add(column.GetText());
                event.Text.Add(s);
            }
        }

        CslPlayerStatsData *data=(CslPlayerStatsData*)GetItemData(item);

        c=CslGeoIP::GetCountryNameByIPnum(data->IP);
        event.Text.Add(_("Country"));
        event.Text.Add(c ? (A2U(c)).c_str() : CslGeoIP::IsOk() ?
                       IsLocalIP(Int2IP(data->IP)) ? T2C(_("local network")) :
                       T2C(_("unknown country")) : T2C(_("GeoIP database not found.")));

        event.Text.Add(wxT("ID / IP"));
        event.Text.Add(wxString::Format(wxT("%d / %d.%d.%d.x"),data->ID,
                                        data->IP>>24,data->IP>>16&0xff,data->IP>>8&0xff));

        event.Title=_("Player information");
    }
}

wxString CslListCtrlPlayer::GetScreenShotFileName()
{
    wxString s;

    s<<m_info->GetBestDescription()<<wxT("-")<<m_info->GameMode<<wxT("-")<<m_info->Map;
    FixFilename(s);
    s<<wxT("-")<<wxDateTime::Now().Format(wxT("%Y%m%d_%H%M%S"))<<wxT(".png");

    return s;
}

wxSize CslListCtrlPlayer::GetImageListSize()
{
    wxInt32 x,y;

    if (ListImageList.GetSize(0,x,y))
        return wxSize(x,y);

    return wxDefaultSize;
}

void CslListCtrlPlayer::UpdateData()
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    if (!m_info)
    {
        ListClear();
        return;
    }

    wxInt32 i,j;
    wxString s;
    wxListItem item;
    CslPlayerStatsData *data=NULL;
    const CslPlayerStats& stats=m_info->PlayerStats;

    j=GetItemCount()-1;

    for (i=0;i<stats.m_stats.length();i++)
    {
        data=stats.m_stats[i];
        if (!data->Ok)
        {
            i--;
            break;
        }

        item.SetId(i);
        if (i>j)
            InsertItem(item);
#ifndef CSL_USE_WX_LIST_DESELECT_WORKAROUND
        else
        {
            m_processSelectEvent=false;

            SetItemState(i,~(wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED),
                         wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

            m_processSelectEvent=true;
        }
#endif //CSL_USE_WX_LIST_DESELECT_WORKAROUND

        SetItemData(i,(long)data);

        if (data->Name.IsEmpty())
            s=_("- connecting -");
        else
            s=data->Name;
        SetItem(i,0,s);

        if (m_view>=SIZE_MINI)
        {
            if (data->State==CSL_PLAYER_STATE_SPECTATOR)
                s=_("Spectator");
            else
                s=data->Team;
            SetItem(i,1,s);

            if (data->Flagscore!=0)
                SetItem(i,2,wxString::Format(wxT("%d / %d"),data->Frags,data->Flagscore));
            else
                SetItem(i,2,wxString::Format(wxT("%d"),data->Frags));

            SetItem(i,3,wxString::Format(wxT("%d"),data->Deaths));

            SetItem(i,4,wxString::Format(wxT("%d"),data->Teamkills));

            if (m_view>=SIZE_DEFAULT)
            {
                if (data->Ping<0)
                    s=_("no data");
                else if (data->Ping>=9999)
                    s=_("LAG");
                else
                    s=wxString::Format(wxT("%d"),data->Ping);
                SetItem(i,5,s);

                SetItem(i,6,wxString::Format(wxT("%d%%"),data->Accuracy));

                if (data->State==CSL_PLAYER_STATE_UNKNOWN)
                    s=_("no data");
                else if (data->State==CSL_PLAYER_STATE_DEAD)
                    s=_("dead");
                else if (data->State==CSL_PLAYER_STATE_EDITING)
                    s=_("editing");
                else if (data->State)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->Health);
                SetItem(i,7,s);

                if (data->State==CSL_PLAYER_STATE_UNKNOWN)
                    s=_("no data");
                else if (data->State || data->Armour<0)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->Armour);
                SetItem(i,8,s);

                s=m_info->GetGame().GetWeaponName(data->Weapon);
                SetItem(i,9,s.IsEmpty() ? wxString(_("no data")):s);
            }
        }

        if (data->Privileges==CSL_PLAYER_PRIV_MASTER)
            SetItemBackgroundColour(item,CSL_COLOUR_MASTER);
        else if (data->Privileges==CSL_PLAYER_PRIV_ADMIN)
            SetItemBackgroundColour(item,CSL_COLOUR_ADMIN);
        else if (data->State==CSL_PLAYER_STATE_SPECTATOR)
            SetItemBackgroundColour(item,CSL_COLOUR_SPECTATOR);
        else
            SetItemBackgroundColour(item,GetBackgroundColour());

        SetItemImage(item,GetCountryFlag(data->IP));
    }

    while (j>i)
    {
        item.SetId(j);
        DeleteItem(item);
        j--;
    }

    if ((i=m_selected.GetCount()))
    {
        while (i>0)
        {
            data=m_selected.Item(--i);

            if ((j=ListFindItem(data,item))==wxNOT_FOUND)
            {
                m_selected.RemoveAt(i);
                delete data;
            }
        }
    }

    ListSort(-1);
}

void CslListCtrlPlayer::EnableEntries(bool enable)
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    if (!m_info)
    {
        ListClear();
        return;
    }

    wxInt32 i;
    wxListItem item;

    for (i=0;i<GetItemCount();i++)
    {
        item.SetId(i);

        if (enable)
            SetItemTextColour(item,wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
        else
            SetItemTextColour(item,wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

void CslListCtrlPlayer::ListClear()
{
    DeleteAllItems();
    WX_CLEAR_ARRAY(m_selected);
}

void CslListCtrlPlayer::ListAdjustSize(const wxSize& size)
{
    if (m_view<0)
        return;

    wxInt32 w=size==wxDefaultSize ? GetClientSize().x-8 : size.x-8;
    if (w<0)
        return;

    if (m_view>=SIZE_DEFAULT)
    {
        SetColumnWidth(0,(wxInt32)(w*0.18f));
        SetColumnWidth(1,(wxInt32)(w*0.09f));
        SetColumnWidth(2,(wxInt32)(w*0.09f));
        SetColumnWidth(3,(wxInt32)(w*0.09f));
        SetColumnWidth(4,(wxInt32)(w*0.09f));
        SetColumnWidth(5,(wxInt32)(w*0.08f));
        SetColumnWidth(6,(wxInt32)(w*0.09f));
        SetColumnWidth(7,(wxInt32)(w*0.09f));
        SetColumnWidth(8,(wxInt32)(w*0.09f));
        SetColumnWidth(9,(wxInt32)(w*0.10f));
    }
    else if (m_view==SIZE_MINI)
    {
#ifdef __WXMSW__
        SetColumnWidth(0,(wxInt32)(w*0.44f));
#else
        SetColumnWidth(0,(wxInt32)(w*0.46f));
#endif
        SetColumnWidth(1,(wxInt32)(w*0.14f));
        SetColumnWidth(2,(wxInt32)(w*0.13f));
        SetColumnWidth(3,(wxInt32)(w*0.13f));
        SetColumnWidth(4,(wxInt32)(w*0.13f));
    }
    else
        SetColumnWidth(0,(wxInt32)(w*1.0f));
}

wxInt32 CslListCtrlPlayer::ListFindItem(CslPlayerStatsData *data,wxListItem& item)
{
    wxInt32 i,j=GetItemCount();

    for (i=0;i<j;i++)
    {
        item.SetId(i);

        if (*data==*(CslPlayerStatsData*)GetItemData(item))
            return i;
    }

    return wxNOT_FOUND;
}

void CslListCtrlPlayer::ListSort(const wxInt32 column)
{
    wxListItem item;
    wxInt32 img,col;

    if (column==-1)
        col=m_sortHelper.Type;
    else
    {
        col=column;
        item.SetMask(wxLIST_MASK_IMAGE);
        GetColumn(col,item);

        if (item.GetImage()==-1 || item.GetImage()==CSL_LIST_IMG_SORT_DSC)
        {
            img=CSL_LIST_IMG_SORT_ASC;
            m_sortHelper.Mode=CslListSortHelper::SORT_ASC;
        }
        else
        {
            img=CSL_LIST_IMG_SORT_DSC;
            m_sortHelper.Mode=CslListSortHelper::SORT_DSC;
        }

        item.Clear();
        item.SetImage(-1);
        SetColumn(m_sortHelper.Type,item);

        item.SetImage(img);
        SetColumn(col,item);

        m_sortHelper.Type=col;
    }

    if (!GetItemCount())
        return;

    m_processSelectEvent=false;

    SortItems(ListSortCompareFunc,(long)&m_sortHelper);

    wxInt32 i,j;

    for (i=0;i<(wxInt32)m_selected.GetCount();i++)
    {
        if ((j=ListFindItem(m_selected.Item(i),item))==wxNOT_FOUND)
            continue;
        SetItemState(j,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
    }

    m_processSelectEvent=true;

#ifndef __WXMSW__
    //removes flicker on autosort for wxGTK and wxMAC
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif
}

void CslListCtrlPlayer::ServerInfo(CslServerInfo *info)
{
    m_info=info;

    ListClear();

    if (!info)
        return;

    wxInt32 i;
    wxListItem item;
    vector<wxString> descriptions;

    info->GetGame().GetPlayerstatsDescriptions(descriptions);

    for (i=0;i<GetColumnCount() && i<descriptions.length();i++)
    {
        GetColumn(i,item);
        item.SetText(descriptions[i]);
        SetColumn(i,item);
    }
}

void CslListCtrlPlayer::ListInit(const wxInt32 view)
{
    wxListItem item;

    m_view=view;

    SetImageList(&ListImageList,wxIMAGE_LIST_SMALL);

    item.SetMask(wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    InsertColumn(0,item);
    SetColumn(0,item);

    if (m_view>=SIZE_MINI)
    {
        item.SetText(_("Team"));
        InsertColumn(1,item);
        SetColumn(1,item);

        item.SetText(_("Frags"));
        InsertColumn(2,item);
        SetColumn(2,item);

        item.SetText(_("Deaths"));
        InsertColumn(3,item);
        SetColumn(3,item);

        item.SetText(_("Teamkills"));
        InsertColumn(4,item);
        SetColumn(4,item);

        if (m_view>=SIZE_DEFAULT)
        {
            item.SetText(_("Ping"));
            InsertColumn(5,item);
            SetColumn(5,item);

            item.SetText(_("Accuracy"));
            InsertColumn(6,item);
            SetColumn(6,item);

            item.SetText(_("Health"));
            InsertColumn(7,item);
            SetColumn(7,item);

            item.SetText(_("Armour"));
            InsertColumn(8,item);
            SetColumn(8,item);

            item.SetText(_("Weapon"));
            InsertColumn(9,item);
            SetColumn(9,item);
        }
    }

    //assertion on __WXMAC__
    //ListAdjustSize();

    wxInt32 img;

    if (m_view==SIZE_MICRO)
        m_sortHelper.Init(CslListSortHelper::SORT_ASC,SORT_NAME);
    else
        m_sortHelper.Init(CslListSortHelper::SORT_DSC,SORT_FRAGS);

    if (m_sortHelper.Mode==CslListSortHelper::SORT_ASC)
        img=CSL_LIST_IMG_SORT_ASC;
    else
        img=CSL_LIST_IMG_SORT_DSC;

    GetColumn(m_sortHelper.Type,item);
    item.SetImage(img);
    SetColumn(m_sortHelper.Type,item);
}

int wxCALLBACK CslListCtrlPlayer::ListSortCompareFunc(long item1,long item2,long data)
{
    CslPlayerStatsData *data1=(CslPlayerStatsData*)item1;
    CslPlayerStatsData *data2=(CslPlayerStatsData*)item2;
    CslListSortHelper *helper=(CslListSortHelper*)data;

    wxInt32 ret=0;

    if (helper->Type!=SORT_NAME)
    {
        if (data1->State==CSL_PLAYER_STATE_SPECTATOR)
            return 1;
        if (data2->State==CSL_PLAYER_STATE_SPECTATOR)
            return -1;
    }

#define LIST_SORT_FRAGS(_data1,_data2,_mode) \
    if (!(ret=helper->Cmp(_data1->Frags,_data2->Frags,_mode))) \
    { \
        if (!(ret=helper->Cmp(_data1->Accuracy,_data2->Accuracy, \
                              CslListSortHelper::SORT_DSC))) \
        { \
            if (!(ret=helper->Cmp(_data1->Deaths,_data2->Deaths, \
                                  CslListSortHelper::SORT_ASC))) \
            { \
                ret=helper->Cmp(_data1->Teamkills,_data2->Teamkills, \
                                CslListSortHelper::SORT_ASC); \
            } \
        } \
    }

    switch (helper->Type)
    {
        case SORT_NAME:
            ret=helper->Cmp(data1->Name,data2->Name);
            break;

        case SORT_TEAM:
            ret=helper->Cmp(data1->Team,data2->Team);
            break;

        case SORT_FRAGS:
            LIST_SORT_FRAGS(data1,data2,helper->Mode);
            break;

        case SORT_DEATHS:
            ret=helper->Cmp(data1->Deaths,data2->Deaths);
            break;

        case SORT_TEAMKILLS:
            ret=helper->Cmp(data1->Teamkills,data2->Teamkills);
            break;

        case SORT_PING:
            ret=helper->Cmp(data1->Ping,data2->Ping);
            break;

        case SORT_ACCURACY:
            ret=helper->Cmp(data1->Accuracy,data2->Accuracy);
            break;

        case SORT_HEALTH:
            ret=helper->Cmp(data1->Health,data2->Health);
            break;

        case SORT_ARMOUR:
            ret=helper->Cmp(data1->Armour,data2->Armour);
            break;

        case SORT_WEAPON:
            ret=helper->Cmp(data1->Weapon,data2->Weapon);
            break;

        default:
            break;
    }

    if (!ret && helper->Type!=SORT_FRAGS)
        LIST_SORT_FRAGS(data1,data2,CslListSortHelper::SORT_DSC);

    return ret;
}
