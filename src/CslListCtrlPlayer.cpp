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
#include "CslListCtrlPlayer.h"
#include "CslDlgConnectPass.h"
#include "CslConnectionState.h"
#include "CslMenu.h"
#include "CslGeoIP.h"
#include "CslFlags.h"
#include "img/sortasc_18_12.xpm"
#include "img/sortdsc_18_12.xpm"

#define CSL_COLOUR_MASTER    wxColour(64,255,128)
#define CSL_COLOUR_ADMIN     wxColour(255,128,0)
#define CSL_COLOUR_SPECTATOR wxColour(192,192,192)

enum
{
    CSL_LIST_IMG_SORT_ASC = 0,
    CSL_LIST_IMG_SORT_DSC,
    CSL_LIST_IMG_UNKNOWN
};

enum
{
    SORT_NAME = 0,
    SORT_TEAM,
    SORT_FRAGS,
    SORT_DEATHS,
    SORT_TEAMKILLS,
    SORT_ACCURACY,
    SORT_HEALTH,
    SORT_ARMOUR,
    SORT_WEAPON
};


BEGIN_EVENT_TABLE(CslListCtrlPlayer,wxListCtrl)
    #ifdef __WXMSW__
    EVT_ERASE_BACKGROUND(CslListCtrlPlayer::OnEraseBackground)
    #endif
    EVT_LIST_COL_CLICK(wxID_ANY,CslListCtrlPlayer::OnColumnLeftClick)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,CslListCtrlPlayer::OnItemActivated)
    EVT_CONTEXT_MENU(CslListCtrlPlayer::OnContextMenu)
    EVT_MENU(wxID_ANY,CslListCtrlPlayer::OnMenu)
    EVT_MOTION(CslListCtrlPlayer::OnMouseMove)
END_EVENT_TABLE()


wxSize CslListCtrlPlayer::BestSizeMicro(140,350);
wxSize CslListCtrlPlayer::BestSizeMini(280,350);
wxImageList CslListCtrlPlayer::ListImageList;
#ifdef __WXMSW__
wxInt32 CslListCtrlPlayer::m_imgOffsetY=0;
#endif


CslListCtrlPlayer::CslListCtrlPlayer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator, const wxString& name)
        : wxListCtrl(parent,id,pos,size,style,validator,name),
        m_view(-1),m_info(NULL)
{
    m_toolTip=new wxToolTip(wxEmptyString);
    SetToolTip(m_toolTip);
}

#ifdef __WXMSW__
void CslListCtrlPlayer::OnEraseBackground(wxEraseEvent& event)
{
    //to prevent flickering, erase only content *outside* of the actual items

    if (GetItemCount()>0)
    {
        wxDC *dc=event.GetDC();

        long i,imgId,topItem,bottomItem;
        wxRect rect1,rect2;
        wxCoord x,y,w,h,width,height;
        wxListItem item;

        GetClientSize(&width,&height);

        dc->SetClippingRegion(0,0,width,height);
        dc->GetClippingBox(&x,&y,&w,&h);

        topItem=GetTopItem();
        bottomItem=topItem+GetCountPerPage();

        if (bottomItem>=GetItemCount())
            bottomItem=GetItemCount()-1;

        GetItemRect(topItem,rect1,wxLIST_RECT_LABEL);
        GetItemRect(bottomItem,rect2,wxLIST_RECT_BOUNDS);

        //set the new clipping region and do erasing
        wxRegion region(x,y,w,h);
        region.Subtract(wxRect(rect1.GetLeftTop(),rect2.GetBottomRight()));

        item.SetMask(wxLIST_MASK_IMAGE);

        for (i=0;i<GetItemCount() && i<=bottomItem;i++)
        {
            item.SetId(i);
            GetItem(item);

            if ((imgId=item.GetImage())<0)
                continue;

            if (!GetItemRect(i,rect1,wxLIST_RECT_ICON))
                continue;

            const wxBitmap& bitmap=ListImageList.GetBitmap(imgId);

            wxRegion imgRegion(bitmap);
            imgRegion.Offset(rect1.x,rect1.y+m_imgOffsetY);
            region.Xor(imgRegion);
        }

        dc->DestroyClippingRegion();
        dc->SetClippingRegion(region);

        //do erasing
        dc->SetBackground(wxBrush(GetBackgroundColour(),wxSOLID));
        dc->Clear();

        //restore old clipping region
        dc->DestroyClippingRegion();
        dc->SetClippingRegion(wxRegion(x,y,w,h));
    }
    else
        event.Skip();
}
#endif

void CslListCtrlPlayer::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslListCtrlPlayer::OnItemActivated(wxListEvent& event)
{
    if (!m_info)
        return;

    CslConnectionState::CreateConnectState(m_info);

    event.Skip();
}

void CslListCtrlPlayer::OnContextMenu(wxContextMenuEvent& event)
{
    if (!m_info)
        return;

    wxMenu menu;
    wxMenuItem *item;
    wxPoint point=event.GetPosition();

    //from keyboard
    if (point.x==-1 && point.y==-1)
        point=wxGetMousePosition();

    CslMenu::AddItem(&menu,MENU_SERVER_CONNECT,MENU_SERVER_CONN_STR,wxART_CONNECT);
    if (CSL_CAP_CONNECT_PASS(m_info->GetGame().GetCapabilities()))
        CslMenu::AddItem(&menu,MENU_SERVER_CONNECT_PW,MENU_SERVER_CONN_PW_STR,wxART_CONNECT_PW);

    if (m_info)
    {
        menu.AppendSeparator();

        wxMenu *ext=new wxMenu();
        item=menu.AppendSubMenu(ext,_("Extended information"));
        item->SetBitmap(GET_ART_MENU(wxART_ABOUT));
        if (m_info->ExtInfoStatus!=CSL_EXT_STATUS_OK || !CslEngine::PingOk(*m_info,g_cslSettings->updateInterval))
            item->Enable(false);
        else
        {
            if (m_view!=CSL_LIST_PLAYER_DEFAULT_SIZE_DLG)
            {
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_FULL,_("Full"),wxART_ABOUT);
                ext->AppendSeparator();
            }
            if (m_view!=CSL_LISTPLAYER_MICRO_SIZE)
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_MICRO,_("Micro"),wxART_EXTINFO_MICRO);
            if (m_view!=CSL_LISTPLAYER_MINI_SIZE)
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_MINI,_("Mini"),wxART_EXTINFO_MINI);
            if (m_view!=CSL_LIST_PLAYER_DEFAULT_SIZE)
                CslMenu::AddItem(ext,MENU_SERVER_EXTENDED_DEFAULT,_("Default"),wxART_EXTINFO_DEFAULT);
        }
    }

    point=ScreenToClient(point);
    PopupMenu(&menu,point);
}

void CslListCtrlPlayer::OnMenu(wxCommandEvent& event)
{
    switch (event.GetId())
    {

        case MENU_SERVER_CONNECT:
            CslConnectionState::CreateConnectState(m_info);
            event.Skip();
            break;

        case MENU_SERVER_CONNECT_PW:
        {
            wxInt32 i=CSL_CAP_CONNECT_ADMIN_PASS(m_info->GetGame().GetCapabilities());

            CslConnectPassInfo pi(m_info->Password,m_info->PasswordAdmin,i!=0);

            if (CslDlgConnectPass(this,&pi).ShowModal()==wxID_OK)
            {
                m_info->Password=pi.Password;
                m_info->PasswordAdmin=pi.AdminPassword;
                i=pi.Admin ? CslGame::CSL_CONNECT_ADMIN_PASS:CslGame::CSL_CONNECT_PASS;
                CslConnectionState::CreateConnectState(m_info,i);
                event.Skip();
            }
            break;
        }
        case MENU_SERVER_EXTENDED_FULL:
        case MENU_SERVER_EXTENDED_MICRO:
        case MENU_SERVER_EXTENDED_MINI:
        case MENU_SERVER_EXTENDED_DEFAULT:
            event.SetClientData(m_info);
            event.Skip();
            break;

        default:
            break;
    }
}

void CslListCtrlPlayer::OnMouseMove(wxMouseEvent& event)
{
    wxRect rect;
    wxListItem item;
    wxString tip;
    wxInt32 i,offset=0;
    wxPoint pos=event.GetPosition();
#ifndef __WXMSW__
    bool first=true;
#endif

    event.Skip();

    if (pos!=wxDefaultPosition)
    {
        for (i=GetTopItem();i<GetItemCount();i++)
        {
            item.SetId(i);
            GetItemRect(item,rect,wxLIST_RECT_BOUNDS);

#ifndef __WXMSW__
            if (first)
            {
                offset=rect.y;
                first=false;
            }
#endif

            rect.y-=offset;

            if (rect.Contains(pos))
            {
                CslPlayerStatsData *data=(CslPlayerStatsData*)GetItemData(item);
                const char *country=CslGeoIP::GetCountryNameByIPnum(data->IP);
                tip=wxString::Format(_("Name: %s   Country: %s"),data->Name.c_str(),
                                     (country ? (A2U(country)).c_str() : CslGeoIP::IsOk() ?
                                      _("Unknown"):_("GeoIP database not found")));
                tip+=wxString::Format(wxT("   ID: %d   IP: %d.%d.%d.x"),data->ID,
                                      data->IP>>24,data->IP>>16&0xff,data->IP>>8&0xff);
                m_toolTip->SetTip(tip);
// arr, undocumented function - totally weird!
#ifdef __WXMAC__
                wxToolTip::RelayEvent(this,event);
#endif
                return;
            }
        }
    }

    tip=_("Player list");
    if (m_info)
        tip+=_(" for server: ")+m_info->GetBestDescription();
    m_toolTip->SetTip(tip);
// arr, undocumented function - totally weird!
#ifdef __WXMAC__
    wxToolTip::RelayEvent(this,event);
#endif
}

void CslListCtrlPlayer::UpdateData()
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    if (!m_info)
    {
        DeleteAllItems();
        return;
    }

    wxInt32 i,j,l;
    wxString s;
    wxListItem item;
    CslPlayerStatsData *data=NULL;
    const CslPlayerStats& stats=m_info->PlayerStats;

    l=GetItemCount()-1;
    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

    for (i=0;i<stats.m_stats.length();i++)
    {
        data=stats.m_stats[i];
        if (!data->Ok)
        {
            i--;
            break;
        }

        item.SetId(i);
        if (i>l)
            InsertItem(item);
        SetItemData(i,(long)data);

        if (data->Name.IsEmpty())
            s=_("- connecting -");
        else
            s=data->Name;
        SetItem(i,0,s);

        if (m_view>=CSL_LISTPLAYER_MINI_SIZE)
        {
            if (data->State==CSL_PLAYER_STATE_SPECTATOR)
                s=_("Spectator");
            else
                s=data->Team;
            SetItem(i,1,s);

            SetItem(i,2,wxString::Format(wxT("%d"),data->Frags));
            SetItem(i,3,wxString::Format(wxT("%d"),data->Deaths));
            SetItem(i,4,wxString::Format(wxT("%d"),data->Teamkills));

            if (m_view>=CSL_LIST_PLAYER_DEFAULT_SIZE)
            {
                SetItem(i,5,wxString::Format(wxT("%d%%"),data->Accuracy));

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
                SetItem(i,6,s);

                if (data->State==CSL_PLAYER_STATE_UNKNOWN)
                    s=_("no data");
                else if (data->State || data->Armour<0)
                    s=wxT("0");
                else
                    s=wxString::Format(wxT("%d"),data->Armour);
                SetItem(i,7,s);

                s=m_info->GetGame().GetWeaponName(data->Weapon);
                SetItem(i,8,s.IsEmpty() ? wxString(_("no data")):s);
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

        const char *country;
        wxInt32 flag=CSL_LIST_IMG_UNKNOWN;

        if (data->IP)
        {
            country=CslGeoIP::GetCountryCodeByIPnum(data->IP);
            if (country)
            {
                for (j=sizeof(codes)/sizeof(codes[0])-1;j>=0;j--)
                {
                    if (!strcasecmp(country,codes[j]))
                    {
                        flag+=++j;
                        break;
                    }
                }
            }
        }

        SetItemImage(item,flag);
    }

    while (l>i)
    {
        item.SetId(l);
        DeleteItem(item);
        l--;
    }

    ListSort(-1);

#ifndef __WXMSW__
    //removes flicker on autosort for wxGTK and wxMAC
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif
}

void CslListCtrlPlayer::ListAdjustSize(const wxSize& size)
{
    if (m_view<0)
        return;

    wxInt32 w=size==wxDefaultSize ? GetClientSize().x-8 : size.x-8;
    if (w<0)
        return;

    if (m_view>=CSL_LIST_PLAYER_DEFAULT_SIZE)
    {
        SetColumnWidth(0,(wxInt32)(w*0.20f));
        SetColumnWidth(1,(wxInt32)(w*0.10f));
        SetColumnWidth(2,(wxInt32)(w*0.10f));
        SetColumnWidth(3,(wxInt32)(w*0.10f));
        SetColumnWidth(4,(wxInt32)(w*0.10f));
        SetColumnWidth(5,(wxInt32)(w*0.10f));
        SetColumnWidth(6,(wxInt32)(w*0.10f));
        SetColumnWidth(7,(wxInt32)(w*0.10f));
        SetColumnWidth(8,(wxInt32)(w*0.10f));
    }
    else if (m_view==CSL_LISTPLAYER_MINI_SIZE)
    {
        SetColumnWidth(0,(wxInt32)(w*0.46f));
        SetColumnWidth(1,(wxInt32)(w*0.14f));
        SetColumnWidth(2,(wxInt32)(w*0.13f));
        SetColumnWidth(3,(wxInt32)(w*0.13f));
        SetColumnWidth(4,(wxInt32)(w*0.13f));
    }
    else
        SetColumnWidth(0,(wxInt32)(w*1.0f));
}

void CslListCtrlPlayer::ListSort(const wxInt32 column)
{
    wxListItem item;
    wxInt32 img;
    wxInt32 col;

    if (column==-1)
        col=m_sortHelper.m_sortType;
    else
    {
        col=column;
        item.SetMask(wxLIST_MASK_IMAGE);
        GetColumn(col,item);

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
        SetColumn(m_sortHelper.m_sortType,item);

        item.SetImage(img);
        SetColumn(col,item);

        m_sortHelper.m_sortType=col;
    }

    if (GetItemCount()>0)
        SortItems(ListSortCompareFunc,(long)&m_sortHelper);
}

void CslListCtrlPlayer::ServerInfo(CslServerInfo *info)
{
    m_info=info;
    DeleteAllItems();

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

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);
    item.SetText(_("Player"));
    InsertColumn(0,item);
    SetColumn(0,item);

    if (m_view>=CSL_LISTPLAYER_MINI_SIZE)
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

        if (m_view>=CSL_LIST_PLAYER_DEFAULT_SIZE)
        {
            item.SetText(_("Accuracy"));
            InsertColumn(5,item);
            SetColumn(5,item);

            item.SetText(_("Health"));
            InsertColumn(6,item);
            SetColumn(6,item);

            item.SetText(_("Armour"));
            InsertColumn(7,item);
            SetColumn(7,item);

            item.SetText(_("Weapon"));
            InsertColumn(8,item);
            SetColumn(8,item);
        }
    }

    //assertion on __WXMAC__
    //ListAdjustSize();

    wxInt32 img;

    if (m_view==CSL_LISTPLAYER_MICRO_SIZE)
        m_sortHelper.Init(CSL_SORT_ASC,SORT_NAME);
    else
        m_sortHelper.Init(CSL_SORT_DSC,SORT_FRAGS);

    if (m_sortHelper.m_sortMode==CSL_SORT_ASC)
        img=CSL_LIST_IMG_SORT_ASC;
    else
        img=CSL_LIST_IMG_SORT_DSC;

    item.SetImage(img);
    SetColumn(m_sortHelper.m_sortType,item);
}

int wxCALLBACK CslListCtrlPlayer::ListSortCompareFunc(long item1,long item2,long data)
{
    CslPlayerStatsData *stats1=(CslPlayerStatsData*)item1;
    CslPlayerStatsData *stats2=(CslPlayerStatsData*)item2;
    wxInt32 sortMode=((CslListSortHelper*)data)->m_sortMode;
    wxInt32 sortType=((CslListSortHelper*)data)->m_sortType;

    if (sortType!=SORT_NAME)
    {
        if (stats1->State==CSL_PLAYER_STATE_SPECTATOR)
            return 1;
        if (stats2->State==CSL_PLAYER_STATE_SPECTATOR)
            return -1;
    }

    wxInt32 type;
    wxInt32 vi1=0,vi2=0;
    wxUint32 vui1=0,vui2=0;
    wxString vs1=wxEmptyString,vs2=wxEmptyString;

    switch (sortType)
    {
        case SORT_NAME:
            type=CSL_LIST_SORT_STRING;
            vs1=stats1->Name;
            vs2=stats2->Name;
            break;

        case SORT_TEAM:
            type=CSL_LIST_SORT_STRING;
            vs1=stats1->Team;
            vs2=stats2->Team;
            break;

        case SORT_FRAGS:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Frags;
            vi2=stats2->Frags;
            break;

        case SORT_DEATHS:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Deaths;
            vi2=stats2->Deaths;
            break;

        case SORT_TEAMKILLS:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Teamkills;
            vi2=stats2->Teamkills;
            break;

        case SORT_ACCURACY:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Accuracy;
            vi2=stats2->Accuracy;
            break;

        case SORT_HEALTH:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Health;
            vi2=stats2->Health;
            break;

        case SORT_ARMOUR:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Armour;
            vi2=stats2->Armour;
            break;

        case SORT_WEAPON:
            type=CSL_LIST_SORT_INT;
            vi1=stats1->Weapon;
            vi2=stats2->Weapon;
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

void CslListCtrlPlayer::CreateImageList()
{
#ifdef __WXMSW__
    //detect vista and set the offset fot the flag images to 2
    //so the region should be correct drawing the background
    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);

    if (ovi.dwMajorVersion>5)
        m_imgOffsetY=2;

    ListImageList.Create(20,14,true);
    ListImageList.Add(AdjustIconSize(sortasc_18_12_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,0)));
    ListImageList.Add(AdjustIconSize(sortdsc_18_12_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,0)));
    ListImageList.Add(AdjustIconSize(unknown_xpm,wxNullIcon,wxSize(20,14),wxPoint(0,2)));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        ListImageList.Add(AdjustIconSize(flags[i],wxNullIcon,wxSize(20,14),wxPoint(0,2)));
#else
    ListImageList.Create(18,12,true);
    ListImageList.Add(wxBitmap(sortasc_18_12_xpm));
    ListImageList.Add(wxBitmap(sortdsc_18_12_xpm));
    ListImageList.Add(wxBitmap(unknown_xpm));

    wxInt32 i,c=sizeof(codes)/sizeof(codes[0])-1;
    for (i=0;i<c;i++)
        ListImageList.Add(wxBitmap(flags[i]));
#endif
}
