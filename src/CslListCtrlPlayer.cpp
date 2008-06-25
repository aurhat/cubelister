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
#include "CslGeoIP.h"
#include "CslFlags.h"
#ifndef __WXMSW__
#include "img/sortasc_18_12.xpm"
#include "img/sortdsc_18_12.xpm"
#endif

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
    EVT_SIZE(CslListCtrlPlayer::OnSize)
    EVT_LIST_COL_CLICK(wxID_ANY,CslListCtrlPlayer::OnColumnLeftClick)
END_EVENT_TABLE()


wxSize CslListCtrlPlayer::BestSizeMicro(140,350);
wxSize CslListCtrlPlayer::BestSizeMini(280,350);

CslListCtrlPlayer::CslListCtrlPlayer(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                     const wxSize& size,long style,
                                     const wxValidator& validator, const wxString& name)
        : wxListCtrl(parent,id,pos,size,style,validator,name)
{
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

void CslListCtrlPlayer::OnSize(wxSizeEvent& event)
{
    ListAdjustSize();
    event.Skip();
}

void CslListCtrlPlayer::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslListCtrlPlayer::ListUpdatePlayerData(CslGame& game,CslPlayerStats& stats)
{
    wxInt32 c;
    wxString s;
    wxListItem item;
    CslPlayerStatsData *data=NULL;

    wxWindowUpdateLocker lock(this);

    DeleteAllItems();

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);

    loopv(stats.m_stats)
    {
        data=stats.m_stats[i];
        if (!data->Ok)
            break;
        if (i<GetItemCount())
            continue;

        item.SetId(i);
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

                s=game.GetWeaponName(data->Weapon);
                SetItem(i,8,s.IsEmpty() ? wxString(_("no data")):s);
            }
        }

        if (data->Privileges==CSL_PLAYER_PRIV_MASTER)
            SetItemBackgroundColour(item,CSL_COLOUR_MASTER);
        else if (data->Privileges==CSL_PLAYER_PRIV_ADMIN)
            SetItemBackgroundColour(item,CSL_COLOUR_ADMIN);
        else if (data->State==CSL_PLAYER_STATE_SPECTATOR)
            SetItemBackgroundColour(item,CSL_COLOUR_SPECTATOR);

        const char *country;
        wxInt32 flag=CSL_LIST_IMG_UNKNOWN;

        if (data->IP && CslGeoIP::IsOk())
        {
            country=CslGeoIP::GetCountryCodeByIPnum(data->IP);
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

        SetItemImage(item,flag);
    }

    ListSort(-1);
}

void CslListCtrlPlayer::ListAdjustSize()
{
    wxInt32 w=GetClientSize().x-8;

    if (m_view==CSL_LIST_PLAYER_DEFAULT_SIZE)
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

void CslListCtrlPlayer::ToggleSortArrow()
{
    wxListItem item;
    wxInt32 img=-1;

    if (m_sortHelper.m_sortMode==CSL_SORT_ASC)
        img=CSL_LIST_IMG_SORT_ASC;
    else
        img=CSL_LIST_IMG_SORT_DSC;

    item.SetImage(img);
    SetColumn(m_sortHelper.m_sortType,item);
}

void CslListCtrlPlayer::SetInfo(CslServerInfo *info)
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

void CslListCtrlPlayer::ListInit(const wxUint32 view)
{
    wxListItem item;

    m_view=view;

    SetImageList(&m_imageList,wxIMAGE_LIST_SMALL);

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

    if (m_view==CSL_LISTPLAYER_MICRO_SIZE)
        m_sortHelper.Init(CSL_SORT_ASC,SORT_NAME);
    else
        m_sortHelper.Init(CSL_SORT_DSC,SORT_FRAGS);

    ToggleSortArrow();
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

