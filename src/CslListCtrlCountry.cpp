/***************************************************************************
 *   Copyright (C) 2007 -2009 by Glen Masgai                               *
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
#include "CslListCtrlCountry.h"
#include "CslGeoIP.h"
#include "engine/CslTools.h"

enum
{
    SORT_NAME = 0,
    SORT_COUNT
};


BEGIN_EVENT_TABLE(CslPanelCountry,wxPanel)
    EVT_SIZE(CslPanelCountry::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CslListCtrlCountry,CslListCtrl)
    EVT_LIST_COL_CLICK(wxID_ANY,CslListCtrlCountry::OnColumnLeftClick)
    EVT_CONTEXT_MENU(CslListCtrlCountry::OnContextMenu)
    EVT_MENU(wxID_ANY,CslListCtrlCountry::OnMenu)
END_EVENT_TABLE()


class CslCountryEntry
{
    public:
        CslCountryEntry(const wxString& country) : Country(country),Count(1) {}

        wxString Country;
        wxUint32 Count;
};


CslPanelCountry::CslPanelCountry(wxWindow* parent,long listStyle) :
        wxPanel(parent,wxID_ANY),
        m_mode(MODE_PLAYER_SINGLE)
{
    m_sizer=new wxFlexGridSizer(2,1,0,0);

    m_listCtrl=new CslListCtrlCountry(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,listStyle);
    m_gauge=new wxGauge(this,wxID_ANY,0,wxDefaultPosition,wxSize(-1,16));
    m_gauge->Hide();

    m_sizer->Add(m_listCtrl,0,wxALL|wxEXPAND,0);
    m_sizer->Add(m_gauge,0,wxALL|wxEXPAND,2);

    m_sizer->AddGrowableRow(0);
    m_sizer->AddGrowableCol(0);

    SetSizer(m_sizer);
    m_sizer->Fit(this);
    m_sizer->Layout();
}

void CslPanelCountry::OnSize(wxSizeEvent& event)
{
#ifdef __WXMAC__
    wxSize size=event.GetSize();
    size.y-=m_gauge->GetBestSize().y+4;
    m_listCtrl->SetSize(size);
#else
    const wxSize& size=event.GetSize();
#endif //__WXMAC__
    m_listCtrl->ListAdjustSize(size);
#ifdef __WXMAC__
    //fixes flicker after resizing
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif //__WXMAC__

    event.Skip();
}

void CslPanelCountry::Reset(wxUint32 mode,wxUint32 count)
{
    m_listCtrl->ListClear();

    if ((m_mode=mode)!=MODE_PLAYER_MULTI)
    {
        m_gauge->Hide();
    }
    else
    {
        m_gauge->Show();
        m_gauge->SetRange(count);
        m_gauge->SetValue(0);
    }

    m_sizer->Layout();
}

void CslPanelCountry::UpdateData(CslServerInfo *info)
{
    m_listCtrl->UpdateData(info);

    if (m_mode==MODE_PLAYER_MULTI)
        m_gauge->SetValue(m_gauge->GetValue()+1);
}


CslListCtrlCountry::CslListCtrlCountry(wxWindow* parent,wxWindowID id,const wxPoint& pos,
                                       const wxSize& size,long style,
                                       const wxValidator& validator,const wxString& name) :
        CslListCtrl(parent,id,pos,size,style,validator,name)
{
    ListInit();
}


CslListCtrlCountry::~CslListCtrlCountry()
{
}

void CslListCtrlCountry::ListInit()
{
    wxListItem item;

    SetImageList(&ListImageList,wxIMAGE_LIST_SMALL);

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_FORMAT);
    item.SetImage(-1);

    item.SetAlign(wxLIST_FORMAT_LEFT);

    item.SetText(_("Country"));
    InsertColumn(0,item);
    SetColumn(0,item);

    item.SetText(_("Count"));
    InsertColumn(1,item);
    SetColumn(1,item);

    m_sortHelper.Init(CSL_SORT_DSC,SORT_COUNT);
    item.SetImage(CSL_LIST_IMG_SORT_DSC);
    SetColumn(m_sortHelper.m_sortType,item);
}

void CslListCtrlCountry::ListAdjustSize(const wxSize& size)
{
    wxInt32 w;

    if ((w=size==wxDefaultSize ? GetClientSize().x-8 : size.x-8)<0)
        return;

    SetColumnWidth(0,(wxInt32)(w*0.70f));
    SetColumnWidth(1,(wxInt32)(w*0.28f));
}

void CslListCtrlCountry::OnColumnLeftClick(wxListEvent& event)
{
    ListSort(event.GetColumn());
}

void CslListCtrlCountry::OnContextMenu(wxContextMenuEvent& event)
{
}

void CslListCtrlCountry::OnMenu(wxCommandEvent& event)
{
}

void CslListCtrlCountry::ListClear()
{
    for (wxInt32 i=GetItemCount()-1;i>=0;i--)
    {
        CslCountryEntry *entry=(CslCountryEntry*)GetItemData(i);

        if (entry)
            delete entry;
    }

    DeleteAllItems();
}

void CslListCtrlCountry::UpdateEntry(const wxString& country,wxInt32 img)
{
    wxInt32 c;
    wxListItem item;
    CslCountryEntry *entry=NULL;

    item.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_IMAGE|wxLIST_MASK_DATA);

    for (c=0;c<GetItemCount();c++)
    {
        item.SetId(c);
        GetItem(item);

        if (img==item.GetImage())
        {
            entry=(CslCountryEntry*)GetItemData(c);
            entry->Count++;
            break;
        }
    }

    if (!entry)
    {
        item.SetId(c);
        InsertItem(item);
        entry=new CslCountryEntry(country.IsEmpty() ? wxString(_("Unknown")) : country);
        SetItemData(c,(long)entry);
    }

    SetItem(c,0,entry->Country,img);
    SetItem(c,1,wxString::Format(wxT("%d"),entry->Count));
}

void CslListCtrlCountry::UpdateData(CslServerInfo *info)
{
    //fixes flickering if scrollbar is shown
    wxWindowUpdateLocker lock(this);

    wxUint32 imgId;
    wxString country;

    if (((CslPanelCountry*)GetParent())->GetMode()==CslPanelCountry::MODE_SERVER)
    {
        if (!info->Pingable)
            return;

        wxUint32 ip=IP2Int(info->Addr.IPAddress());

        if (ip)
        {
            imgId=GetCountryFlag(ip);
            country=A2U(CslGeoIP::GetCountryNameByIPnum(ip));

            UpdateEntry(country,imgId);
        }
    }
    else
    {
        CslPlayerStatsData *player;

        loopv(info->PlayerStats.m_stats)
        {
            if (!(player=info->PlayerStats.m_stats[i])->Ok)
                break;

            imgId=GetCountryFlag(player->IP);
            country=A2U(CslGeoIP::GetCountryNameByIPnum(player->IP));

            UpdateEntry(country,imgId);
        }
    }

    ListSort(-1);

#ifndef __WXMSW__
    //removes flicker on autosort for wxGTK and wxMAC
    wxIdleEvent idle;
    wxTheApp->SendIdleEvents(this,idle);
#endif
}

void CslListCtrlCountry::ListSort(const wxInt32 column)
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

int wxCALLBACK CslListCtrlCountry::ListSortCompareFunc(long item1,long item2,long data)
{
    CslCountryEntry *entry1=(CslCountryEntry*)item1;
    CslCountryEntry *entry2=(CslCountryEntry*)item2;
    wxInt32 sortMode=((CslListSortHelper*)data)->m_sortMode;
    wxInt32 sortType=((CslListSortHelper*)data)->m_sortType;

    wxInt32 type;
    wxUint32 vui1=0,vui2=0;
    wxString vs1=wxEmptyString,vs2=wxEmptyString;

    switch (sortType)
    {
        case SORT_NAME:
            type=CSL_LIST_SORT_STRING;
            vs1=entry1->Country;
            vs2=entry2->Country;
            break;

        case SORT_COUNT:
            type=CSL_LIST_SORT_UINT;
            vui1=entry1->Count;
            vui2=entry2->Count;
            break;

        default:
            return 0;
    }

    if (type==CSL_LIST_SORT_UINT)
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
